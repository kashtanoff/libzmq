#pragma once

// fxconf.cpp: определяет экспортированные функции для приложения DLL.

#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>

#include <codecvt>

#include <ctime>
#include <thread>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "Defines.h"

#include "fxc/fxc.h"
#include "fxc/ConnectionAdapter.cpp"

#include INCLUDE_FILE(STRAT_PATH)

#if DEBUG
#include <chrono>
#endif

std::vector<STRAT_CLASS*> pool;
__declspec(thread) STRAT_CLASS* strategy;
bool isRunAllowed = true;

struct AccountOrder {
	int        ticket;
	__time64_t opentime;
	__time64_t closetime;
	double     profit;
};

struct AccountData {
	double      balance;
	double      equity;
	double      profit;
	int         status;
	std::time_t lastUpdate = 0;
	std::time_t lastSync   = 0;
} account;

std::vector<AccountOrder> openedOrders;
std::vector<AccountOrder> serverOrders;

std::string resolveError(int err) {
	switch (err) {
		//NO_BREAK
		case 0:	  return "all ok";
		//SOFT_BREAK
		case 1:   return "negative balance";
		case 2:   return "update required";
		//HARD_BREAK
		case 100: return "invalid broker";
		case 101: return "banned broker";
		case 200: return "invalid account";
		case 201: return "banned account";
		case 202: return "invalid account info";
		case 300: return "invalid socket";
		case 301: return "server is not accessible";
		case 500: return "unknown";
	}
	return "blocked by server";
}
std::string resolveStatus(int status) {
	switch (status) {
		case STATUS_OK:              return "trading is allowed";
		case STATUS_DANGER:	         return "Attention!";
		case STATUS_SOFT_BREAK:      return "finalizing trading";
		case STATUS_HARD_BREAK:      return "trading is not allowed";
		case STATUS_EMERGENCY_BREAK: return "Emergecy stop!";
		default: return "Unknown status";
	}
}

const std::string getRequestJson(STRAT_CLASS* expert) {
	auto tester = expert->mqlTester || expert->mqlOptimization;
	std::stringstream ss;

	ss << "{"
		<< "\"tester\":"    <<  tester                     << ","
		<< "\"magic\":"     <<  expert->expertMagic        << ","
		<< "\"expert\":"    << (expert->expertMagic >> 16) << ","
		<< "\"version\":\"" <<  EXPERT_VERSION             << "\",";

#if PARTNER_ID
	ss << "\"partner\":" << PARTNER_ID << ",";
#endif

	if (!tester) {
		account.lastUpdate = 0;
		ss
			<< "\"balance\":" << account.balance       << ","
			<< "\"equity\":"  << account.equity        << ","
			<< "\"profit\":"  << account.profit        << ","
			<< "\"orders\":[";

		int n = 0;
		for (int i = 0; i < openedOrders.size(); i++, n++) {
			if (n > 0) {
				ss << ",";
			}

			ss << "{"
				<< "\"ticket\":" << openedOrders[i].ticket << ","
				<< "\"opened\":" << openedOrders[i].opentime << ","
				<< "\"closed\":0,"
				<< "\"profit\":0"
			<< "}";
		}

		for (int i = 0; i < serverOrders.size(); i++, n++) {
			if (n > 0) {
				ss << ",";
			}

			ss << "{"
				<< "\"ticket\":" << serverOrders[i].ticket    << ","
				<< "\"opened\":" << serverOrders[i].opentime  << ","
				<< "\"closed\":" << serverOrders[i].closetime << ","
				<< "\"profit\":" << serverOrders[i].profit    << ""
			<< "}";
		}
		
		ss << "],";
	}
	ss
		<< "\"leverage\":"   <<  expert->accountLeverage        <<   ","
		<< "\"demo\":"       << (expert->accountTradeMode == 0) <<   ","
		<< "\"number\":"     <<  expert->accountNumber          <<   ","
		<< "\"name\":\""     <<  expert->accountName            << "\","
		<< "\"broker\":\""   <<  expert->accountCompany         << "\","
		<< "\"server\":\""   <<  expert->accountServer          << "\","
		<< "\"currency\":\"" <<  expert->accountCurrency        << "\""
	<< "}";

	return ss.str();
}

bool isGlobalWorkersAllowed = true;
bool isAccessWorkerActive   = false;

int         work_status = STATUS_HARD_BREAK;
std::string reason      = "not initialized";

void checkAccess() {
#if CHECK_ACCESS
	#if DEBUG
		try {
	#endif
			// прошло менее 1 часа и не появились новые ордера
			if (!openedOrders.size() && time(0) - account.lastSync < 60 * 60) {
				return;
			}

			fxc::mutex.lock();
			STRAT_CLASS* expert = nullptr;

			for (auto& entry : pool) {
				expert = entry;

				if (!expert->mqlTester && !expert->mqlOptimization) {
					break;
				}
			}

			if (expert == nullptr) {
				expert = strategy;
			}
			// В этом потоке может не быть strategy
			if (expert == nullptr) {
				return;
			}

			auto request = getRequestJson(expert);
			fxc::mutex.unlock();

			fxc::ConnectionAdapter connection(
#if LOCAL
				"tcp://localhost:13857"
#else
				"tcp://term.olsencleverton.com:13857"
#endif
			);
			if (connection.send(request)) {
				rapidjson::Document doc;

				if (!doc.Parse(connection.getResponse().c_str()).HasParseError()) {
				#pragma region Valid renponse

					account.lastSync = time(0);
					account.status   = doc["statusCode"].GetInt();
					reason           = doc["status"].GetString();
					
					switch (account.status) {
						case 0:
							work_status = STATUS_OK;
							break;
						case 1:
						case 2:
						case 3:
							work_status = STATUS_SOFT_BREAK;
							break;
						default:
							work_status = STATUS_HARD_BREAK;
							break;
					}

					fxc::mutex.lock();
					serverOrders.clear();

					auto orders = doc.FindMember("orders");
					if (orders != doc.MemberEnd()) {
						for (auto itr = orders->value.Begin(); itr != orders->value.End(); ++itr) {
							serverOrders.push_back(AccountOrder{ itr->GetInt(), 0, 0, 0 });

							for (auto openItr = openedOrders.begin(); openItr != openedOrders.end(); ++openItr) {
								if (openItr->ticket == itr->GetInt()) {
									openedOrders.erase(openItr);
									break;
								}
							}
						}
					}
					fxc::mutex.unlock();

					rapidjson::Value::MemberIterator message = doc.FindMember("message");
					if (message != doc.MemberEnd()) {
						std::wstring_convert< std::codecvt_utf8_utf16<wchar_t> > converter;

						auto text = message->value["text"].GetString();
						auto url  = message->value.FindMember("url");

						if (IDOK == MessageBox(0, 
							converter.from_bytes(text).c_str(), 
							converter.from_bytes(EXPERT_NAME).c_str(), 
							url != message->value.MemberEnd() ? MB_OKCANCEL : MB_OK
						) && url != message->value.MemberEnd()) {
							ShellExecute(NULL, L"open", converter.from_bytes(url->value.GetString()).c_str(), NULL, NULL, SW_SHOWNORMAL);
						}

						(void) url;
					}
					(void) message;

				#pragma endregion
				}
				else {
					work_status = STATUS_SOFT_BREAK;
					reason      = "invalid response";
					fxc::msg << "!> can't parse response\r\n" << fxc::msg_box;
				}
			}
			else {
				switch (connection.getErrType()) {
					case CONN_ERRT_SOCK:
						work_status = STATUS_SOFT_BREAK;
						reason      = "invalid socket";
						break;
					case CONN_ERRT_SEND:
						work_status = STATUS_SOFT_BREAK;
						reason      = "server is not accessible";
						break;
					case CONN_ERRT_RECV:
						work_status = STATUS_SOFT_BREAK;
						reason      = "server is not accessible";
						break;
					default:
						work_status = STATUS_SOFT_BREAK;
						reason      = "network error";
						break;
				}
			}


			fxc::mutex.lock();
			for (auto& entry : pool) {
				if (entry->mqlTester || entry->mqlOptimization) {
					entry->setStatus(PROVIDER_SERVER, STATUS_OK, "test is allowed", reason);
				}
				else {
					entry->setStatus(PROVIDER_SERVER, work_status, resolveStatus(work_status), reason);
				}
			}
			fxc::mutex.unlock();
	#if DEBUG
		}
		catch (const std::exception& ex) {
			fxc::msg << "!> ERROR @ checkAccessWorker(): " << ex.what() << "\r\n" << fxc::msg_box;
			fxc::msg << STACK_TRACE << fxc::msg_box;
		}
		catch (const std::string& ex) {
			fxc::msg << "!> ERROR @ checkAccessWorker(): " << ex << "\r\n" << fxc::msg_box;
			fxc::msg << STACK_TRACE << fxc::msg_box;
		}
		catch (...) {
			fxc::msg << "!> ERROR @ checkAccessWorker(): [undefined type]\r\n" << fxc::msg_box;
			fxc::msg << STACK_TRACE << fxc::msg_box;
		}
	#endif
#else
fxc::mutex.lock();
for (auto& entry : pool) {
	entry->setStatus(PROVIDER_SERVER, STATUS_OK, "no server", "connection off");
}
fxc::mutex.unlock();
#endif
}
void checkAccessWorker()
{
	int timeout;
	STACK_TRACE_INIT
	isAccessWorkerActive = true;
	account.status       = 500;
	account.lastSync     = 0;
	
#if CHECK_ACCESS
	while (isGlobalWorkersAllowed) {
		fxc::msg << "-> checkAccessWorker()\r\n" << fxc::msg_box;
		checkAccess();
		for (int i = 0; isGlobalWorkersAllowed && i < 600; i++) 	{  //Это для того, чтобы в случае выгрузки длл быстро выйти
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
#else
	checkAccess();
#endif

	fxc::msg << "~> checkAccessWorker()\r\n" << fxc::msg_box;
	isAccessWorkerActive = false;
	STACK_TRACE_FLUSH
}

std::thread checkAccessThread;

#pragma region Интерфейс с MQL

#pragma region Var binding

_DLLAPI void __stdcall c_setint(wchar_t* propName, int propValue)
{
	fxc::mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto prop = &strategy->PropertyList[name];
	
	if (prop->Type == PropInt) {
		*(prop->Int) = propValue;
	}
	else {
		fxc::msg << "c_setint ERROR: int expected - " << name << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setdouble(wchar_t* propName, double propValue)
{
	fxc::mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto prop = &strategy->PropertyList[name];

	if (prop->Type == PropDouble) {
		*(prop->Double) = propValue;
	}
	else {
		fxc::msg << "c_setdouble ERROR: double expected" << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setstring(wchar_t* propName, wchar_t* propValue)
{
	fxc::mutex.lock();

	auto name = fxc::utils::WC2MB(propName);
	auto prop = &strategy->PropertyList[name];

	if (prop->Type == PropString) {
		*(prop->String) = std::string(fxc::utils::WC2MB(propValue));
	}
	else if (prop->Type == PropWString) {
		*(prop->WString) = std::wstring(propValue);
	}
	else {
		fxc::msg << "c_setstring ERROR: string or wstring expected" << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setvar(wchar_t* propName, void* pointer)
{
	fxc::mutex.lock();

	char name[64];
	wcstombs(name, propName, 32);

	auto prop = &strategy->PropertyList[name];

	if (prop->Type == PropIntPtr) {
		*(prop->IntPtr)    = (int*) pointer;
	}
	else if (prop->Type == PropLongPtr) {
		*(prop->LongPtr)   = (long*) pointer;
	}
	else if (prop->Type == PropDoublePtr) {
		*(prop->DoublePtr) = (double*) pointer;
	}
	else if (prop->Type == PropBoolPtr) {
		*(prop->BoolPtr)   = (bool*) pointer;
	}
	else {
		fxc::msg << "c_setvar ERROR: int, double or bool pointer expected" << fxc::msg_box;
	}

	fxc::mutex.unlock();
}

_DLLAPI void __stdcall c_setactions(void* pointer, int length) {
	fxc::mutex.lock();
	strategy->mapActions(pointer, length);
	fxc::mutex.unlock();
}

#pragma endregion

_DLLAPI bool __stdcall c_init()
{
	STACK_TRACE_INIT

#if DEBUG
	try {
		if (AllocConsole()) { // Создаем консоль, у процесса не более одной.
			// Связываем буферы консоли с предопределенными файловыми описателями.
			freopen("conin$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("conout$", "w", stderr);
			fxc::console = true;
			fxc::msg << "Debug logging activated\r\n" << fxc::msg_box;
		}
#endif
		MARK_FUNC_IN
		fxc::mutex.lock();
		
		strategy = new STRAT_CLASS();
		pool.push_back(strategy);

		fxc::mutex.unlock();
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

_DLLAPI void __stdcall c_set_usermagic(int usermagic) {
	usermagic = usermagic % 0xff;
	strategy->expertMagic = MAGIC_OC | MAGIC_EA | usermagic;
}

_DLLAPI void __stdcall c_postInit() {
#if DEBUG
	try {
#endif
		MARK_FUNC_IN
		strategy->init();
		//strategy->setStatus(PROVIDER_SERVER, work_status, reason);

		fxc::mutex.lock();
		isGlobalWorkersAllowed = true;

		fxc::msg << "-> isGlobalWorkersAllowed status: " << isGlobalWorkersAllowed << "; AccessWorker status: " << isAccessWorkerActive << "\r\n" << fxc::msg_box;

		// Если глобальные потоки еще не запущены, то запускаем
		// Если же уже есть один поток, то второй нам не нужен
		if (!isAccessWorkerActive) {
			checkAccessThread = std::thread(checkAccessWorker);
			checkAccessThread.detach();
		}
		else {
			if (strategy->mqlTester || strategy->mqlOptimization) {
				strategy->setStatus(PROVIDER_SERVER, STATUS_OK, "test is allowed", reason);
			}
			else {
				strategy->setStatus(PROVIDER_SERVER, work_status, resolveStatus(work_status), reason);
			}
		}
		fxc::mutex.unlock();

		////TEMP: временная конструкция, пока не поймем причину отказа воркера
		//if (strategy->mqlTester || strategy->mqlOptimization) {
		//	strategy->setStatus(PROVIDER_SERVER, STATUS_OK, "test is allowed", reason);
		//}

		//int timer = 20;
		//if (strategy->breakStatus >= STATUS_SOFT_BREAK) {
		//	fxc::msg << "Wait permissions... (" << isGlobalWorkersAllowed << ")\r\n" << fxc::msg_box;
		//}

		//while (timer-- > 0 && strategy->breakStatus >= STATUS_SOFT_BREAK) {
		//	std::this_thread::sleep_for(std::chrono::seconds(1));
		//}
		//
		//if (strategy->breakStatus >= STATUS_SOFT_BREAK) {
		//	fxc::msg << "Permissions not recived!!! (" << isGlobalWorkersAllowed << "), work_status: " << work_status << "\r\n" << fxc::msg_box;
		//}

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
	fxc::mutex.lock();
	auto iter = std::find(pool.begin(), pool.end(), strategy);
	if (iter != pool.end()) {
		pool.erase(iter);
	}
	else {
		fxc::msg << "c_deinit strategy instance not found\r\n" << fxc::msg_box;
	}
	auto lastInstance = !pool.size();
	fxc::mutex.unlock();

	// Если у нас больше не осталось экземпляров советника, то можно прекращать передачу данных серверу
	if (lastInstance) {
		isGlobalWorkersAllowed = false;
		//checkAccess();  //Вслучае выгрузки длл надо быстро выйти иначе терминал упадет
	}

	delete strategy;
	strategy = nullptr;

	STACK_TRACE_FLUSH
}

_DLLAPI int __stdcall c_updateAccount(double balance, double equity, double profit) {
	if (!strategy->mqlTester && !strategy->mqlOptimization) {
		fxc::mutex.lock();
		account.balance    = balance;
		account.equity     = equity;
		account.profit     = profit;
		account.lastUpdate = time(0);
		fxc::mutex.unlock();
	}

	return serverOrders.size() ? serverOrders.begin()->ticket : 0;
}

_DLLAPI int __stdcall c_updateOrder(int ticket, __time64_t opentime, __time64_t closetime, double profit) {
	fxc::mutex.lock();
	int next = 0;
	auto itr = serverOrders.begin();
	for (itr = serverOrders.begin(); itr != serverOrders.end(); ++itr) {
		if (itr->ticket == ticket) {
			itr->opentime  = opentime;
			itr->closetime = closetime;
			itr->profit    = profit;

			if (++itr != serverOrders.end()) {
				next = itr->ticket;
			}

			break;
		}
	}
	fxc::mutex.unlock();

	return next;
}

_DLLAPI void __stdcall c_onOrderOpen(int ticket, __time64_t opentime) {
	fxc::mutex.lock();
	openedOrders.push_back(AccountOrder{ ticket, opentime });
	fxc::mutex.unlock();
}

//Выполняет этап алгоритма, возвращает индекс необходимого действия
_DLLAPI int __stdcall c_getjob()
{
	auto res = 0;

#if DEBUG
	try {
#endif
		MARK_FUNC_IN
		res = strategy->getJob();
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

	return res;
}

//Выдает текущее разрешение экрана (для правильного масштабирования графики)
_DLLAPI int __stdcall c_getdpi()
{
	__try {
		auto hDC  = ::GetDC(nullptr); 
		auto nDPI = ::GetDeviceCaps(hDC, LOGPIXELSX); 
		ReleaseDC(nullptr, hDC);
		return nDPI;
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
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
		strategy->getChartData(timeframe)->update((MqlRates*) pointer, length);
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
	auto timeframes = strategy->getTimeframes();
	auto length     = timeframes.size();

	for (int i = 0; i < length; ++i) {
		*((int*) timeframesPtr + i) = timeframes[i];
		*((int*) sizesPtr      + i) = strategy->getChartData(timeframes[i])->getSize();
	}

	return length;
}

//Инициализация цикла обновления ордеров, поскольку цикл получаеться рваным (каждая итерация вызывается из MQL), 
//нельзя пользоватья результатами в MQL программе до завершения цикла
_DLLAPI bool __stdcall c_tick_init_begin(double ask, double bid, double equity, double balance)
{
	STACK_TRACE_CLEAR
	auto res = strategy->tickInitBegin(ask, bid, equity, balance);
	return res;
}

_DLLAPI void __stdcall c_tick_init_end()
{
	DEBUG_TRY
	MARK_FUNC_IN
	strategy->tickInitEnd();
	MARK_FUNC_OUT
	DEBUG_CATCH("c_tick_init_end()")
}

//Добавляет новый ордер в цикле скана ордеров, в будущем возвращает код изменения
_DLLAPI int __stdcall c_add_order(int _ticket, int _magic, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0)
{
	int result;
	DEBUG_TRY
	result = strategy->addOrder(_ticket, _magic, _type, _lots, _openprice, _tp, _sl, _profit);
	DEBUG_CATCH("c_add_order()")
	return result;
}

//Нормализация лота для ручных операций
_DLLAPI double __stdcall c_norm_lot(double _lots)
{
	double result;
	DEBUG_TRY
	result = strategy->normLot(_lots);
	DEBUG_CATCH("c_norm_lot()")
	return result;
}

_DLLAPI double __stdcall c_get_ontester()
{
	return strategy->getOnTester();
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