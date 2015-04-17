#pragma once

// fxconf.cpp: определяет экспортированные функции для приложения DLL.

#include "zmq.h"

#include <ctime>
#include <thread>
#include <unordered_map>

#include "stdafx.h"
#include "Defines.h"

#include "fxc/fxc.h"

#include INCLUDE_FILE(STRAT_PATH)

#if DEBUG
#include <chrono>
#endif

std::unordered_map<std::thread::id, STRAT_CLASS*> pool;
bool isRunAllowed = true;

struct AccountData {
	double      balance;
	double      equity;
	double      profit;
	int         status;
	std::time_t lastUpdate;
} account;

bool isGlobalWorkersAllowed = true;
bool isAccessWorkerActive   = false;
void checkAccessWorker()
{
	isAccessWorkerActive = true;
	std::this_thread::sleep_for(std::chrono::seconds(5));

	while (isGlobalWorkersAllowed) {
		fxc::msg << "-> checkAccessWorker()\r\n" << fxc::msg_box;

		auto& expert = pool.begin()->second;
		if (!expert->mqlTester && !expert->mqlOptimization) {

#if CHECK_ACCESS
			auto isBlock    = false;
			auto isChanged  = false;
			auto context    = zmq_ctx_new();
			auto socket     = zmq_socket(context, ZMQ_REQ);
			auto connectErr = zmq_connect(socket, 
#if LOCAL
				"tcp://localhost:13857"
#else
				"tcp://olsencleverton.com:13857"
#endif
				);
		
			std::stringstream ss;
			ss
				<< "{"
					<< "\"balance\":"    <<  account.balance                << ","
					<< "\"equity\":"     <<  account.equity                 << ","
					<< "\"profit\":"     <<  account.profit                 << ","
					<< "\"leverage\":"   <<  expert->accountLeverage        << ","
					<< "\"demo\":"       << (expert->accountTradeMode == 0) << ","
					<< "\"number\":"     <<  expert->accountNumber          << ","
					<< "\"name\":\""     <<  expert->accountName            << "\","
					<< "\"broker\":\""   <<  expert->accountCompany         << "\","
					<< "\"server\":\""   <<  expert->accountServer          << "\","
					<< "\"currency\":\"" <<  expert->accountCurrency        << "\""
				<< "}";

			std::string message = ss.str();

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
					fxc::msg << "-> send error: " << zmq_errno() << "\r\n" << fxc::msg_box;
					isBlock = true;
				}

				char buffer[64];
				auto recvSize = zmq_recv(socket, &buffer, sizeof(buffer), 0);
				if (-1 == recvSize) {
					fxc::msg << "-> receive error: " << zmq_errno() << "\r\n" << fxc::msg_box;
					isBlock = true;
				}
				else {
					auto response = std::string(buffer).substr(0, recvSize);
					fxc::msg << "-> received: [" << response << "]\r\n" << fxc::msg_box;

					try {
						auto code = std::stoi(response);
						isChanged = account.status != code;
						account.status = code;

						fxc::msg << "-> parsed: [" << code << "]\r\n" << fxc::msg_box;
					}
					catch (std::exception e) {
						fxc::msg << "-> parse error: [" << e.what() << "]\r\n" << fxc::msg_box;
					}
				}

				if (-1 == zmq_close(socket)) {
					fxc::msg << "-> close error: " << zmq_errno() << "\r\n" << fxc::msg_box;
				}

				if (-1 == zmq_ctx_term(context)) {
					fxc::msg << "-> terminate error: " << zmq_errno() << "\r\n" << fxc::msg_box;
				}
			}
			else {
				fxc::msg << "-> connect error: " << connectErr << "\r\n" << fxc::msg_box;
				isBlock = true;
			}

			if (isBlock || (isChanged && account.status > 0)) {
				// TODO: описать стратегию прекращения работы

				//isRunAllowed = false;
				//for (auto &pair : pool) {
				//	*(pair.second->ext_isRunAllowed) = isRunAllowed;
				//}
				break;
			}
#endif

		}

		account.lastUpdate = 0;
		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
	fxc::msg << "~> checkAccessWorker()\r\n" << fxc::msg_box;
	isAccessWorkerActive = false;
}

std::thread checkAccessThread;

#pragma region Интерфейс с MQL

#pragma region Var binding

_DLLAPI void __stdcall c_setint(wchar_t* propName, int propValue)
{
	fxc::mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto h    = std::this_thread::get_id();
	auto prop = &pool[h]->PropertyList[name];
	
	if (prop->Type == PropInt) {
		*(prop->Int) = propValue;
	} else {
		fxc::msg << "c_setint ERROR: int expected - " << name << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setdouble(wchar_t* propName, double propValue)
{
	fxc::mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto h    = std::this_thread::get_id();
	auto prop = &pool[h]->PropertyList[name];

	if (prop->Type == PropDouble) {
		*(prop->Double) = propValue;
	} else {
		fxc::msg << "c_setdouble ERROR: double expected" << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setstring(wchar_t* propName, wchar_t* propValue)
{
	fxc::mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);
	char buffer[512];
	wcstombs(buffer, propValue, 256);

	auto h = std::this_thread::get_id();
	auto prop = &pool[h]->PropertyList[name];

	if (prop->Type == PropString) {
		*(prop->String) = std::string(buffer);
	}
	else {
		fxc::msg << "c_setstring ERROR: string expected" << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setvar(wchar_t* propName, void* pointer)
{
	fxc::mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto h    = std::this_thread::get_id();
	auto prop = &pool[h]->PropertyList[name];

	if (prop->Type == PropIntPtr) {
		*(prop->IntPtr)   = (int*) pointer;
		//fxc::msg << "pointer <" << name << ">: simbiot::[" << *(prop->IntPtr) << "] mql::[" << pointer << "]" << "\r\n" << fxc::msg_box;
	} else if (prop->Type == PropLongPtr) {
		*(prop->LongPtr) = (long*) pointer;
		//fxc::msg << "pointer <" << name << ">: simbiot::[" << *(prop->LongPtr) << "] mql::[" << pointer << "]" << "\r\n" << fxc::msg_box;
	} else if (prop->Type == PropDoublePtr) {
		*(prop->DoublePtr) = (double*) pointer;
		//fxc::msg << "pointer <" << name << ">: simbiot::[" << *(prop->DoublePtr) << "] mql::[" << pointer << "]" << "\r\n" << fxc::msg_box;
	} else if (prop->Type == PropBoolPtr) {
		*(prop->BoolPtr) = (bool*) pointer;
		//fxc::msg << "pointer <" << name << ">: simbiot::[" << *(prop->BoolPtr) << "] mql::[" << pointer << "]" << "\r\n" << fxc::msg_box;
	} else {
		fxc::msg << "c_setvar ERROR: int, double or bool pointer expected" << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setactions(void* pointer, int length) {
	fxc::mutex.lock();
	//fxc::msg << "-> c_actions([0x" << pointer << "], " << length << ")\r\n" << fxc::msg_box;
	pool[std::this_thread::get_id()]->mapActions(pointer, length);
	fxc::mutex.unlock();
}

#pragma endregion

//Ищет свободную ячейку в пуле и создает новый экземпляр симбиота
_DLLAPI bool __stdcall c_init()
{
#if DEBUG
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
		fxc::console = true;
		fxc::msg << "Debug logging activated\r\n" << fxc::msg_box;
	}
#endif

#if DEBUG
	try {
#endif
		MARK_FUNC_IN
		fxc::mutex.lock();
		pool[std::this_thread::get_id()] = new STRAT_CLASS ();
		fxc::mutex.unlock();

		isGlobalWorkersAllowed = true;
		// Если глобальные потоки еще не запущены, то запускаем
		// Если же уже есть один поток, то второй нам не нужен
		if (!isAccessWorkerActive) {
			checkAccessThread = std::thread(checkAccessWorker);
			checkAccessThread.detach();
		}
		MARK_FUNC_OUT
#if DEBUG
	}
	catch (const std::exception& ex) {
		fxc::msg << "!> ERROR @ c_init(): " << ex.what() << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (const std::string& ex) {
		fxc::msg << "!> ERROR @ c_init(): " << ex << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (...) {
		fxc::msg << "!> ERROR @ c_init(): [undefined type]\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
#endif

	return true;
}

_DLLAPI void __stdcall c_postInit() {
#if DEBUG
	try {
#endif
		MARK_FUNC_IN
		pool[std::this_thread::get_id()]->init();
		MARK_FUNC_OUT
#if DEBUG
	}
	catch (const std::exception& ex) {
		fxc::msg << "!> ERROR @ c_postInit(): " << ex.what() << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (const std::string& ex) {
		fxc::msg << "!> ERROR @ c_postInit(): " << ex << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (...) {
		fxc::msg << "!> ERROR @ c_postInit(): [undefined type]\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
#endif
}

//Освобождает память и индекс в пуле
_DLLAPI void __stdcall c_deinit()
{
	pool.erase(std::this_thread::get_id());

	// Если у нас больше не осталось экземпляров советника, то можно прекращать передачу данных серверу
	if (!pool.size()) {
		isGlobalWorkersAllowed = false;
	}
}

_DLLAPI void __stdcall c_updateAccount(double balance, double equity, double profit) {
	account.balance    = balance;
	account.equity     = equity;
	account.profit     = profit;
	account.lastUpdate = time(0);
}

//Выполняет этап алгоритма, возвращает индекс необходимого действия
_DLLAPI int __stdcall c_getjob()
{
#if PROFILE
	auto t1 = std::chrono::high_resolution_clock::now();
#endif

	auto res = 0;
	auto   h = std::this_thread::get_id();

#if DEBUG
	try {
#endif
		MARK_FUNC_IN
		res = pool[h]->getJob();
		MARK_FUNC_OUT
#if DEBUG
	}
	catch (const std::exception& ex) {
		fxc::msg << "!> ERROR @ c_getjob(): " << ex.what() << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (const std::string& ex) {
		fxc::msg << "!> ERROR @ c_getjob(): " << ex << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (...) {
		fxc::msg << "!> ERROR @ c_getjob(): [undefined type]\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
#endif

#if PROFILE 
	auto t2 = std::chrono::high_resolution_clock::now();
	auto  d = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	if (d > 0) {
		fxc::msg << "$> duration @ c_getjob(): " << d << "\r\n" << fxc::msg_box;
	}
#endif

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
		fxc::msg << "c_getdpi ERROR: " << GetExceptionCode() << fxc::msg_box;
	}

	return 1;
}

_DLLAPI void __stdcall c_refresh_chartdata(int timeframe, int length, void* pointer)
{
#if DEBUG
	try {
#endif
		MARK_FUNC_IN
		pool[std::this_thread::get_id()]->getChartData(timeframe)->update((MqlRates*) pointer, length);
		MARK_FUNC_OUT
		return;
#if DEBUG
	}
	catch (const std::exception& ex) {
		fxc::msg << "!> ERROR @ c_refresh_chartdata(): " << ex.what() << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (const std::string& ex) {
		fxc::msg << "!> ERROR @ c_refresh_chartdata(): " << ex << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (...) {
		fxc::msg << "!> ERROR @ c_refresh_chartdata(): [undefined type]\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
#endif
}

_DLLAPI int __stdcall c_get_timeframes(void* timeframesPtr, void* sizesPtr)
{
	auto as = pool[std::this_thread::get_id()];
	auto timeframes = as->getTimeframes();
	auto length     = timeframes.size();

	for (int i = 0; i < length; ++i) {
		*((int*) timeframesPtr + i) = timeframes[i];
		*((int*) sizesPtr + i)      = as->getChartData(timeframes[i])->getSize();
	}

	return length;
}

//Инициализация цикла обновления ордеров, поскольку цикл получаеться рваным (каждая итерация вызывается из MQL), 
//нельзя пользоватья результатами в MQL программе до завершения цикла
_DLLAPI bool __stdcall c_tick_init_begin(double ask, double bid, double equity, double balance)
{
	return pool[std::this_thread::get_id()]->tickInitBegin(ask, bid, equity, balance);
}

_DLLAPI void __stdcall c_tick_init_end()
{
#if DEBUG
	try {
#endif
		MARK_FUNC_IN
		pool[std::this_thread::get_id()]->tickInitEnd();
		MARK_FUNC_OUT
		return;
#if DEBUG
	}
	catch (const std::exception& ex) {
		fxc::msg << "!> ERROR @ c_tick_init_end(): " << ex.what() << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (const std::string& ex) {
		fxc::msg << "!> ERROR @ c_tick_init_end(): " << ex << "\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
	catch (...) {
		fxc::msg << "!> ERROR @ c_tick_init_end(): [undefined type]\r\n" << fxc::msg_box;
		fxc::msg << STACK_TRACE << fxc::msg_box;
	}
#endif
}

//Добавляет новый ордер в цикле скана ордеров, в будущем возвращает код изменения
_DLLAPI int __stdcall c_add_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0)
{
	return pool[std::this_thread::get_id()]->addOrder(_ticket, _type, _lots, _openprice, _tp, _sl, _profit);
}

//Нормализация лота для ручных операций
_DLLAPI double __stdcall c_norm_lot(double _lots)
{
	return pool[std::this_thread::get_id()]->normLot(_lots);
}

_DLLAPI int __stdcall c_get_next_closed()
{
	return pool[std::this_thread::get_id()]->getNextClosedTicket();
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