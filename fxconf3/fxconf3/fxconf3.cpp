#pragma once

// fxconf.cpp: определяет экспортированные функции для приложения DLL.

#include "zmq.h"

#include <thread>
#include <unordered_map>

#include "Defines.h"
#include "fxc/fxc.h"
#include "fxc/Simbiot.cpp"

using namespace fxc;

std::unordered_map<std::thread::id, Simbiot*> pool;
bool isRunAllowed = true;

int         accNumber = 0;
std::string accServer;

bool isWorkerAllowed = true;
bool isWorkerActive  = false;
void checkAccessWorker()
{
	isWorkerActive = true;
	while (isWorkerAllowed) {
		msg << "-> checkAccessWorker()\r\n" << msg_box;

		auto isBlock    = false;
		auto context    = zmq_ctx_new();
		auto socket     = zmq_socket(context, ZMQ_REQ);
		auto connectErr = zmq_connect(socket, "tcp://localhost:13857");

		std::string message("");
		message
			.append(std::to_string(accNumber))
			.append("@")
			.append(accServer);

		auto linger     = 2000;
		auto sndTimeout = 4000;
		auto rcvTimeout = 4000;
		auto correlate  = true;
		zmq_setsockopt(socket, ZMQ_LINGER,        &linger,     sizeof(linger));
		zmq_setsockopt(socket, ZMQ_SNDTIMEO,      &sndTimeout, sizeof(sndTimeout));
		zmq_setsockopt(socket, ZMQ_RCVTIMEO,      &rcvTimeout, sizeof(rcvTimeout));
		zmq_setsockopt(socket, ZMQ_REQ_CORRELATE, &correlate,  sizeof(correlate));

		if (0 == connectErr) {
			if (-1 == zmq_send(socket, message.c_str(), strlen(message.c_str()), 0)) {
				msg << "-> send error: " << zmq_errno() << "\r\n" << msg_box;
				isBlock = true;
			}

			char buffer[64];
			auto recvSize = zmq_recv(socket, &buffer, sizeof(buffer), 0);
			if (-1 == recvSize) {
				msg << "-> receive error: " << zmq_errno() << "\r\n" << msg_box;
				isBlock = true;
			} else {
				auto response = std::string(buffer).substr(0, recvSize);
				msg << "-> received: [" << response << "]\r\n" << msg_box;

				try {
					auto code = std::stoi(response);
					if (code != 1) {
						isBlock = true;
					}
					msg << "-> parsed: [" << code << "]\r\n" << msg_box;
				} catch (std::exception e) {
					msg << "-> parse error: [" << e.what() << "]\r\n" << msg_box;
				}
			}

			if (-1 == zmq_close(socket)) {
				msg << "-> close error: " << zmq_errno() << "\r\n" << msg_box;
			}

			if (-1 == zmq_ctx_term(context)) {
				msg << "-> terminate error: " << zmq_errno() << "\r\n" << msg_box;
			}
		} else {
			msg << "-> connect error: " << connectErr << "\r\n" << msg_box;
			isBlock = true;
		}

		if (isBlock) {
			isRunAllowed = false;
			for (auto &pair : pool) {
				*(pair.second->isRunAllowed) = isRunAllowed;
			}
			break;
		}

		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
	msg << "~> checkAccessWorker()\r\n" << msg_box;
	isWorkerActive = false;
}

std::thread checkAccessThread;

#pragma region Интерфейс с MQL

#pragma region Var binding

_DLLAPI void __stdcall c_setint(wchar_t* propName, int propValue)
{
	mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto h    = std::this_thread::get_id();
	auto prop = &pool[h]->PropertyList[name];
	
	if (prop->Type == PropInt) {
		*(prop->Int) = propValue;
	} else {
		bp = true;
		msg << "c_setint ERROR: int expected" << msg_box;
	}

	mutex.unlock();
}

_DLLAPI void __stdcall c_setdouble(wchar_t* propName, double propValue)
{
	mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto h    = std::this_thread::get_id();
	auto prop = &pool[h]->PropertyList[name];

	if (prop->Type == PropDouble) {
		*(prop->Double) = propValue;
	} else {
		bp = true;
		msg << "c_setdouble ERROR: double expected" << msg_box;
	}

	mutex.unlock();
}

_DLLAPI void __stdcall c_setvar(wchar_t* propName, void* pointer)
{
	mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto h    = std::this_thread::get_id();
	auto prop = &pool[h]->PropertyList[name];

	if (prop->Type == PropIntPtr) {
		*(prop->IntPtr)   = (int*) pointer;
		//msg << "pointer <" << name << ">: simbiot::[" << *(prop->IntPtr) << "] mql::[" << pointer << "]" << "\r\n" << msg_box;
	} else if (prop->Type == PropDoublePtr) {
		*(prop->DoublePtr) = (double*) pointer;
		//msg << "pointer <" << name << ">: simbiot::[" << *(prop->DoublePtr) << "] mql::[" << pointer << "]" << "\r\n" << msg_box;
	} else if (prop->Type == PropBoolPtr) {
		*(prop->BoolPtr) = (bool*) pointer;
		//msg << "pointer <" << name << ">: simbiot::[" << *(prop->PropBoolPtr) << "] mql::[" << pointer << "]" << "\r\n" << msg_box;
	} else {
		bp = true;
		msg << "c_setvar ERROR: int, double ot bool pointer expected" << msg_box;
	}

	mutex.unlock();
}

#pragma endregion

//Ищет свободную ячейку в пуле и создает новый экземпляр симбиота
_DLLAPI bool __stdcall c_init(int accountNumber, wchar_t* accountServer, char* symbol)
{
	char buffer[512];
	wcstombs(buffer, accountServer, 256);

	if (accNumber == 0) {
		accNumber = accountNumber;
		accServer = std::string(buffer);
	} else if (accNumber != accountNumber || accServer != std::string(buffer)) {
		return false;
	}

	mutex.lock();

	if (DEBUG == 1)
	{
		/*
		RedirectIOToConsole();
		msg << "Debug logging activated\r\n" << msg_box;
		*/

		if (AllocConsole()) { // Создаем консоль, у процесса не более одной.
			//SetConsoleTitle("Debug Console");
			// Связываем буферы консоли с предопределенными файловыми описателями.
			freopen("conin$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("conout$", "w", stderr);
			console = true;
			msg << "Debug logging activated\r\n" << msg_box;
		}
	}

	auto h = std::this_thread::get_id();
	//bp = true;
	//msg << "Pool position: " << h << msg_box;
	pool[h] = new Simbiot(symbol);
	mutex.unlock();

	if (!isWorkerActive) {
		isWorkerAllowed   = true;
		checkAccessThread = std::thread(checkAccessWorker);
		checkAccessThread.detach();
	}

	return true;
}

_DLLAPI void __stdcall c_postInit() {
	auto h = std::this_thread::get_id();
	pool[h]->PostInit();
}

//Освобождает память и индекс в пуле
_DLLAPI void __stdcall c_deinit()
{
	if (isWorkerActive) {
		isWorkerAllowed = false;
	}

	mutex.lock();
	pool.erase( std::this_thread::get_id() );
	//FreeConsole();
	mutex.unlock();
}

//Выполняет этап алгоритма, возвращает индекс необходимого действия
_DLLAPI int __stdcall c_getjob()
{
	mutex.lock();
	//curSymbol = pool[curH]->symbol;
	auto   h = std::this_thread::get_id();
	auto res = pool[h]->getjob();
	mutex.unlock();

	return res;
}

//Выдает текущее разрешение экрана (для правильного масштабирования графики)
_DLLAPI int __stdcall c_getdpi()
{
	__try
	{
		HDC hDC  = ::GetDC(nullptr); 
		int nDPI = ::GetDeviceCaps(hDC, LOGPIXELSX); 
		ReleaseDC(nullptr, hDC);
		return nDPI;
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		bp = true;
		msg << "c_getdpi ERROR: " << GetExceptionCode() << msg_box;
	};

	return 1;
}

//Инициализация цикла обновления ордеров, поскольку цикл получаеться рваным (каждая итерация вызывается из MQL), 
//нельзя пользоватья результатами в MQL программе до завершения цикла
_DLLAPI void __stdcall c_refresh_init(double ask, double bid, double equity)   
{
	mutex.lock();
	auto h = std::this_thread::get_id();
	pool[h]->refresh_init(ask, bid, equity);
	mutex.unlock();
}

//Добавляет новый ордер в цикле скана ордеров, в будущем возвращает код изменения
_DLLAPI int __stdcall c_refresh_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0)
{
	mutex.lock();
	auto   h = std::this_thread::get_id();
	auto res = pool[h]->refresh_order(_ticket, _type, _lots, _openprice, _tp, _sl, _profit);
	mutex.unlock();

	return res;
}

//Нормализация лота для ручных операций
_DLLAPI double __stdcall c_norm_lot(double _lots)
{
	//mutex.lock();	
	auto   h = std::this_thread::get_id();
	auto res = pool[h]->normlot(_lots);
	//mutex.unlock();

	return res;
}

_DLLAPI int __stdcall c_get_closed()
{
	//mutex.lock();
	auto   h = std::this_thread::get_id();
	auto res = pool[h]->getclosed();
	//mutex.unlock();

	return res;
}

_DLLAPI void __stdcall c_refresh_prices(double *_closes, double *_highs, double *_lows, int _bars)
{
	//mutex.lock();
	auto h = std::this_thread::get_id();
	pool[h]->refresh_prices(_closes, _highs, _lows, _bars);
	//mutex.unlock();
}

_DLLAPI void __stdcall fcostransform(double a[], int tnn, int inversefct)
{
    int j;
    int n2;
    double sum;
    double y1;
    double y2;
    double theta;
    double wi;
    double wpi;
    double wr;
    double wpr;
    double wtemp;
    double twr;
    double twi;
    double twpr;
    double twpi;
    double twtemp;
    double ttheta;
    int i;
    int i1;
    int i2;
    int i3;
    int i4;
    double c1;
    double c2;
    double h1r;
    double h1i;
    double h2r;
    double h2i;
    double wrs;
    double wis;
    int nn;
    int ii;
    int jj;
    int n;
    int mmax;
    int m;
    int istep;
    int isign;
    double tempr;
    double tempi;

    if( tnn==1 )
    {
        y1 = a[0];
        y2 = a[1];
        a[0] = 0.5*(y1+y2);
        a[1] = 0.5*(y1-y2);
        if( inversefct )
        {
            a[0] += a[0];
            a[1] += a[1];
        }
        return;
    }
    wi = 0;
    wr = 1;
    theta = PI/tnn;
    wtemp = sin(theta*0.5);
    wpr = -2.0*wtemp*wtemp;
    wpi = sin(theta);
    sum = 0.5*(a[0]-a[tnn]);
    a[0] = 0.5*(a[0]+a[tnn]);
    n2 = tnn+2;
    for(j = 2; j <= tnn/2; j++)
    {
        wtemp = wr;
        wr = wtemp*wpr-wi*wpi+wtemp;
        wi = wi*wpr+wtemp*wpi+wi;
        y1 = 0.5*(a[j-1]+a[n2-j-1]);
        y2 = a[j-1]-a[n2-j-1];
        a[j-1] = y1-wi*y2;
        a[n2-j-1] = y1+wi*y2;
        sum = sum+wr*y2;
    }
    ttheta = 2.0*PI/tnn;
    c1 = 0.5;
    c2 = -0.5;
    isign = 1;
    n = tnn;
    nn = tnn/2;
    j = 1;
    for(ii = 1; ii <= nn; ii++)
    {
        i = 2*ii-1;
        if( j>i )
        {
            tempr = a[j-1];
            tempi = a[j];
            a[j-1] = a[i-1];
            a[j] = a[i];
            a[i-1] = tempr;
            a[i] = tempi;
        }
        m = n/2;
        while(m>=2&&j>m)
        {
            j = j-m;
            m = m/2;
        }
        j = j+m;
    }
    mmax = 2;
    while(n>mmax)
    {
        istep = 2*mmax;
        theta = 2.0*PI/(isign*mmax);
        wpr = -2.0*pow(sin(0.5*theta),2);
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for(ii = 1; ii <= mmax/2; ii++)
        {
            m = 2*ii-1;
            for(jj = 0; jj <= (n-m)/istep; jj++)
            {
                i = m+jj*istep;
                j = i+mmax;
                tempr = wr*a[j-1]-wi*a[j];
                tempi = wr*a[j]+wi*a[j-1];
                a[j-1] = a[i-1]-tempr;
                a[j] = a[i]-tempi;
                a[i-1] = a[i-1]+tempr;
                a[i] = a[i]+tempi;
            }
            wtemp = wr;
            wr = wr*wpr-wi*wpi+wr;
            wi = wi*wpr+wtemp*wpi+wi;
        }
        mmax = istep;
    }
    twpr = -2.0*pow(sin(0.5*ttheta),2);
    twpi = sin(ttheta);
    twr = 1.0+twpr;
    twi = twpi;
    for(i = 2; i <= tnn/4+1; i++)
    {
        i1 = i+i-2;
        i2 = i1+1;
        i3 = tnn+1-i2;
        i4 = i3+1;
        wrs = twr;
        wis = twi;
        h1r = c1*(a[i1]+a[i3]);
        h1i = c1*(a[i2]-a[i4]);
        h2r = -c2*(a[i2]+a[i4]);
        h2i = c2*(a[i1]-a[i3]);
        a[i1] = h1r+wrs*h2r-wis*h2i;
        a[i2] = h1i+wrs*h2i+wis*h2r;
        a[i3] = h1r-wrs*h2r+wis*h2i;
        a[i4] = -h1i+wrs*h2i+wis*h2r;
        twtemp = twr;
        twr = twr*twpr-twi*twpi+twr;
        twi = twi*twpr+twtemp*twpi+twi;
    }
    h1r = a[0];
    a[0] = h1r+a[1];
    a[1] = h1r-a[1];
    a[tnn] = a[1];
    a[1] = sum;
    j = 4;
    while(j<=tnn)
    {
        sum = sum+a[j-1];
        a[j-1] = sum;
        j = j+2;
    }
    if( inversefct )
    {
        for(j = 0; j <= tnn; j++)
        {
            a[j] = a[j]*2/tnn;
        }
    }
    return;
}

#pragma endregion