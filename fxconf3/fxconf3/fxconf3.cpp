#pragma once

// fxconf.cpp: ���������� ���������������� ������� ��� ���������� DLL.

#include "zmq.h"

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

int         accNumber = 0;
std::string accServer;

bool isWorkerAllowed = true;
bool isWorkerActive  = false;
void checkAccessWorker()
{
	isWorkerActive = true;
	while (isWorkerAllowed) {
		fxc::msg << "-> checkAccessWorker()\r\n" << fxc::msg_box;

#if CHECK_ACCESS
		auto isBlock    = false;
		auto context    = zmq_ctx_new();
		auto socket     = zmq_socket(context, ZMQ_REQ);
		auto connectErr = zmq_connect(socket, 
#if LOCAL
			"tcp://localhost:13857"
#else
			"tcp://olsencleverton.com:13857"
#endif
		);

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
				fxc::msg << "-> send error: " << zmq_errno() << "\r\n" << fxc::msg_box;
				isBlock = true;
			}

			char buffer[64];
			auto recvSize = zmq_recv(socket, &buffer, sizeof(buffer), 0);
			if (-1 == recvSize) {
				fxc::msg << "-> receive error: " << zmq_errno() << "\r\n" << fxc::msg_box;
				isBlock = true;
			} else {
				auto response = std::string(buffer).substr(0, recvSize);
				fxc::msg << "-> received: [" << response << "]\r\n" << fxc::msg_box;

				try {
					auto code = std::stoi(response);
					if (code != 1) {
						isBlock = true;
					}
					fxc::msg << "-> parsed: [" << code << "]\r\n" << fxc::msg_box;
				} catch (std::exception e) {
					fxc::msg << "-> parse error: [" << e.what() << "]\r\n" << fxc::msg_box;
				}
			}

			if (-1 == zmq_close(socket)) {
				fxc::msg << "-> close error: " << zmq_errno() << "\r\n" << fxc::msg_box;
			}

			if (-1 == zmq_ctx_term(context)) {
				fxc::msg << "-> terminate error: " << zmq_errno() << "\r\n" << fxc::msg_box;
			}
		} else {
			fxc::msg << "-> connect error: " << connectErr << "\r\n" << fxc::msg_box;
			isBlock = true;
		}

		if (isBlock) {
			isRunAllowed = false;
			for (auto &pair : pool) {
				*(pair.second->ext_isRunAllowed) = isRunAllowed;
			}
			break;
		}

#endif

		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
	fxc::msg << "~> checkAccessWorker()\r\n" << fxc::msg_box;
	isWorkerActive = false;
}

std::thread checkAccessThread;

#pragma region ��������� � MQL

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
		fxc::msg << "c_setint ERROR: int expected" << fxc::msg_box;
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

//���� ��������� ������ � ���� � ������� ����� ��������� ��������
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

	fxc::mutex.lock();

	if (DEBUG == 1)
	{
		/*
		RedirectIOToConsole();
		msg << "Debug logging activated\r\n" << msg_box;
		*/

		if (AllocConsole()) { // ������� �������, � �������� �� ����� �����.
			//SetConsoleTitle("Debug Console");
			// ��������� ������ ������� � ����������������� ��������� �����������.
			freopen("conin$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("conout$", "w", stderr);
			fxc::console = true;
			fxc::msg << "Debug logging activated\r\n" << fxc::msg_box;
		}
	}

	auto h = std::this_thread::get_id();
	//bp = true;
	//msg << "Pool position: " << h << msg_box;
	pool[h] = new STRAT_CLASS (symbol);
	fxc::mutex.unlock();

	if (!isWorkerActive) {
		isWorkerAllowed   = true;
		checkAccessThread = std::thread(checkAccessWorker);
		checkAccessThread.detach();
	}

	return true;
}

_DLLAPI void __stdcall c_postInit() {
	auto h = std::this_thread::get_id();
	pool[h]->postInit();
}

//����������� ������ � ������ � ����
_DLLAPI void __stdcall c_deinit()
{
	if (isWorkerActive) {
		isWorkerAllowed = false;
	}

	fxc::mutex.lock();
	pool.erase( std::this_thread::get_id() );
	//FreeConsole();
	fxc::mutex.unlock();
}

//��������� ���� ���������, ���������� ������ ������������ ��������
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
		fxc::mutex.lock();
		res = pool[h]->getJob();
		fxc::mutex.unlock();
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

//������ ������� ���������� ������ (��� ����������� ��������������� �������)
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

//������������� ����� ���������� �������, ��������� ���� ����������� ������ (������ �������� ���������� �� MQL), 
//������ ����������� ������������ � MQL ��������� �� ���������� �����
_DLLAPI void __stdcall c_refresh_init(double ask, double bid, double equity)   
{
	fxc::mutex.lock();
	auto h = std::this_thread::get_id();
	pool[h]->refresh_init(ask, bid, equity);
	fxc::mutex.unlock();
}

//��������� ����� ����� � ����� ����� �������, � ������� ���������� ��� ���������
_DLLAPI int __stdcall c_add_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0)
{
	fxc::mutex.lock();
	auto   h = std::this_thread::get_id();
	auto res = pool[h]->addOrder(_ticket, _type, _lots, _openprice, _tp, _sl, _profit);
	fxc::mutex.unlock();

	return res;
}

//������������ ���� ��� ������ ��������
_DLLAPI double __stdcall c_norm_lot(double _lots)
{
	//fxc::mutex.lock();	
	auto   h = std::this_thread::get_id();
	auto res = pool[h]->normLot(_lots);
	//fxc::mutex.unlock();

	return res;
}

_DLLAPI int __stdcall c_get_next_closed()
{
	//fxc::mutex.lock();
	auto   h = std::this_thread::get_id();
	auto res = pool[h]->getNextClosedTicket();
	//fxc::mutex.unlock();

	return res;
}

_DLLAPI void __stdcall c_refresh_prices(double *_closes, double *_highs, double *_lows, int _bars)
{
	//fxc::mutex.lock();
	auto h = std::this_thread::get_id();
	pool[h]->refresh_prices(_closes, _highs, _lows, _bars);
	//fxc::mutex.unlock();
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