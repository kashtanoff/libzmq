//+------------------------------------------------------------------+
//|                                                          {{ver}}   |
//|                                  Copyright 2015, Olsen&Cleverton |
//|                                       http://olsencleverton.com/ |
//+------------------------------------------------------------------+
#property copyright "Copyright 2015, Olsen&Cleverton"
#property link      "http://olsencleverton.com/"
#property version   "{{ver}}"  
#property icon      "logo.ico"

#resource "logo.bmp"

int ocMagic  = 0x7ED80000;
int eaMagic  = 0x00000000;
int cfgMagic = 0x00000000;
int magic    = ocMagic | eaMagic | cfgMagic;

#include "Inputs.mq4"

struct TradeAction {
	double o_lots;      // 8 bytes
	double o_openprice; // 8 bytes
	double o_tpprice;   // 8 bytes
	double o_slprice;   // 8 bytes
	int    o_ticket;    // 4 bytes
	int    o_type;      // 4 bytes
	int    intret;      // 4 bytes
	int    actionId;    // 4 bytes
	string comment;     // 12 bytes
};

struct TfRates {
	int timeframe;
	int length;
	MqlRates rates[];
};

string ExtractString(string str) {
	int len = StringLen(str);
	for (int i = 0; i < len; i++) {
		if (StringGetChar(str, i) == 0) {
			return StringSubstr(str, 0, i);
		}
	}
	return str;
}

#import "ocsingle.dll"
	bool   c_init();
	void   c_deinit();
	void   c_postInit();

	void   c_setint(string prop, int value);
	void   c_setdouble(string prop, double value);
	void   c_setstring(string prop, string value);
	void   c_setvar(string prop, bool& var);
	void   c_setvar(string prop, int& var);
	void   c_setvar(string prop, int& var[]);
	void   c_setvar(string prop, long& var);
	void   c_setvar(string prop, double& var);
	void   c_setvar(string prop, double& var[]);

	void   c_setactions(TradeAction& var[], int length);
	int    c_updateAccount(double balance, double equity, double profit);
	int    c_updateOrder(int ticket, datetime opentime, datetime closetime, double profit);
	void   c_onOrderOpen(int ticket, datetime opentime);

	int    c_add_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0);
	double c_norm_lot(double _lots);
	int    c_getjob();
	int    c_getdpi();
	int    c_get_next_closed();

	void   c_refresh_chartdata(int timeframe, int length, MqlRates& rates[]);
	int    c_get_timeframes(int& timeframes[], int& sizes[]);
	bool   c_tick_init_begin(double ask, double bid, double equity, double balance);
	void   c_tick_init_end();
#import

#include "Errors.mq4"
#include "Order.mq4"
#include "Graph.mq4"
#include "Look.mq4"

//----- Глобальные переменные и определения -----
int tfs[7] = {PERIOD_M1, PERIOD_M5, PERIOD_M15, PERIOD_M30, PERIOD_H1, PERIOD_H4, PERIOD_D1};
int k;      //Коэффициент пересчета старых пунктов в новые
//Кеширование статических характеристик
int    mqlOptimization;
int    mqlTester;
int    mqlVisualMode;
string symbolName;
double symbolLotStep;
double symbolMarginRequired;

//----- Общие с DLL переменные
int    count[2];
double open_dd[2];
double total_lots[2];
int    max_lvl;
double max_dd;
double indicator;
double indicator2;
double prev_ind;

TradeAction actList[64];
TfRates     tfRates[];
int         tfCount = 0;

int      timeout;
datetime lastAccountUpdateTime;

bool   showinfo;
bool   ecn_mode;
bool   ShowDebug = false;

bool   show_cpanel = false;
double kdpi;
string lotnames[2];
color  colors[6] = {clrDodgerBlue, clrRed, clrNONE, clrNONE, clrNONE, clrNONE};

int    herror, hddlog;
double daydd;
int    lastday;
datetime lastdate;
int    ticks;
string reason = "";
datetime lastbar;
datetime startdate[2];
int netlevel[2];
datetime enddate[2];
int lastlevel[2]= {0, 0};
double netdd[2] = {0, 0};

//+------------------------------------------------------------------+
//| Live cycle:                                                      |
//|                                                                  |
//| OnInit()   =>                                                    |
//| OnTick()   =>                                                    |
//| OnTick()   =>                                                    |
//| ...        =>                                                    |
//| OnTick()   =>                                                    |
//| OnTester() =>                                                    |
//| OnDeinit()                                                       |
//+------------------------------------------------------------------+

Look info(10, clrWhite, 10, 16);

int OnInit()
{
	k = (Digits == 3 || Digits == 5) ? 10 : 1;  //Приведение параметров к дельте цены с переводом в старые пункты
	mqlOptimization       = IsOptimization();
	mqlTester             = IsTesting();
	mqlVisualMode         = IsVisualMode();
	symbolName            = Symbol();
	symbolLotStep         = MarketInfo(symbolName, MODE_LOTSTEP);
	symbolMarginRequired  = MarketInfo(symbolName, MODE_MARGINREQUIRED);
	
	showinfo    = (mqlOptimization || (mqlTester && !mqlVisualMode)) ? false : true; 
	ecn_mode    = (mqlOptimization || mqlTester) ? false : true;

	if (showinfo) InitLook();
	if (!IsDllsAllowed()) CriticalError("DLL is not allowed");
	if (!DllInit()) return (INIT_FAILED);

	UpdateAccountInfo();

	hddlog = FileOpen("dd.log", FILE_WRITE|FILE_TXT);
	FileSeek(hddlog, 0, SEEK_END);
	herror = FileOpen("dd_err.log", FILE_WRITE|FILE_TXT);
	FileSeek(herror, 0, SEEK_END);
	FileWrite(herror, "======= Start =======");

	max_dd   = 0;
	max_lvl  = 0;
	daydd    = 0;
	lastday  = Day();
	lastdate = TimeCurrent();

	EventSetTimer(1);
	Print("tfCount=", tfCount);
	return (INIT_SUCCEEDED);
}

void OnTick()
{
	if (c_tick_init_begin(Ask, Bid, AccountEquity(), AccountBalance())) {
		Print("bypass");
		return;
	}

	for (int i = 0; i < tfCount; i++) {
		// TfRates tf = tfRates[i];  //кэширование индексации (индексация работает в mql медленно)
		// Если последний переданный бар уже не последний, то передаем данные заного
		if (tfRates[i].rates[tfRates[i].length - 1].time != iTime(NULL, tfRates[i].timeframe, 0)) {
			if (CopyRates(NULL, tfRates[i].timeframe, 0, tfRates[i].length, tfRates[i].rates) < tfRates[i].length) {
				// Если скопировано не все, то сбрасываем тик
				Print("CopyRates error");
				return;
			}
			else {
				c_refresh_chartdata(tfRates[i].timeframe, tfRates[i].length, tfRates[i].rates);
			}
		}
		else {
			// Если для меньшего таймфрема мы не прошли границу таймфрема, нет смысла проверять ее для более высоких таймфреймов
			break;
		}
	}
	c_tick_init_end();
	UpdateOrders(); // Обновляем информацию об ордерах

	int l = c_getjob();
	for (i = 0; i < l; i++) {
		//Print("-> Job {", 
		//	"o_lots: ",      actList[i].o_lots,      ", ",
		//	"o_openprice: ", actList[i].o_openprice, ", ",
		//	"o_tpprice: ",   actList[i].o_tpprice,   ", ",
		//	"o_slprice: ",   actList[i].o_slprice,   ", ",
		//	"o_ticket: ",    actList[i].o_ticket,    ", ",
		//	"o_type: ",      actList[i].o_type,      ", ",
		//	"intret: ",      actList[i].intret,      ", ",
		//	"actionId: ",    actList[i].actionId, 
		//"}");
		switch (actList[i].actionId) {
			case 1: Order::Create    (actList[i]); break;
			case 2: Order::Modify    (actList[i]); break;
			case 3: Order::Delete    (actList[i]); break;
			case 4: Order::Close     (actList[i]); break;
			case 5: Order::PrintOrder(actList[i]); break;
			case 6: Order::PrintText (actList[i]); break;
			case 7: Order::DrawOrder (actList[i]); break;
			case 8: Order::ShowValue (actList[i]); break;
			case 9: Order::MsgBox    (actList[i]); break;
		}
	}
	Sleep(timeout);
}

void OnDeinit(const int r)
{
   ObjectsDeleteAll();
   Print("Deinit, ", ticks, " ticks done");
   c_deinit();
   FileClose(herror);
   FileClose(hddlog);
}

double OnTester()
{
	return (TesterStatistics(STAT_PROFIT) / fmax(1, TesterStatistics(STAT_EQUITY_DD)));
}

void OnTimer()
{
	if (TimeGMT() - lastAccountUpdateTime >= 60) {
		UpdateAccountInfo();
	}
}

void OnChartEvent(const int id, const long& lparam, const double& dparam, const string& sparam)
{
	if (id == CHARTEVENT_OBJECT_CLICK)
	{
		double buylot  = BaseBuyLot;
		double selllot = BaseSellLot;
		double curlots = fmin(BaseBuyLot, BaseSellLot);

		if (show_cpanel)
		{
			curlots = StrToDouble(ObjectGetString(0, "EdtLot", OBJPROP_TEXT));
			buylot  = StrToDouble(ObjectGetString(0, "EdtBuyLot", OBJPROP_TEXT));
			selllot = StrToDouble(ObjectGetString(0, "EdtSellLot", OBJPROP_TEXT));
		}

		if (sparam == "BtnMinus") {
			curlots = curlots - symbolLotStep - 0.001;
		}
		else if (sparam == "BtnPlus") {
			curlots = curlots + symbolLotStep;
		}
		else if (sparam == "EdtBuyLot") {
			curlots = buylot;
		}
		else if (sparam == "EdtSellLot") {
			curlots = selllot;
		}
		else if (sparam == "BtnBuy") {
			TradeAction action;

			action.o_type      = OP_BUY;
			action.o_lots      = c_norm_lot(curlots);
			action.o_openprice = Ask;
			action.o_slprice   = Ask - StopLoss * k * Point;
			action.o_tpprice   = Ask + TakeProfit * k * Point;
			action.intret      = 100;

			Order::Create(action);
		}
		else if (sparam == "BtnSell") {
			TradeAction action;

			action.o_type      = OP_SELL;
			action.o_lots      = c_norm_lot(curlots);
			action.o_openprice = Bid;
			action.o_slprice   = Bid + StopLoss * k * Point;
			action.o_tpprice   = Bid - TakeProfit * k * Point;
			action.intret      = 100;
			
			Order::Create(action);
		}
		else if (sparam == "BtnClsB") {
			//CloseAllProfit(OP_BUY);
		}
		else if (sparam == "BtnClsS") {
			//CloseAllProfit(OP_SELL);
		}
		else if (sparam == "textbox") {
			if (show_cpanel) {
				DelControlPanel();
			} else {
				ShowControlPanel(info.x + info.dx + 10, info.y);
			}
		}
	}
}

//+------------------------------------------------------------------+
//| Прочие функции                                                   |
//+------------------------------------------------------------------+

bool DllInit()
{
	c_init();
	c_setstring("accountCompany",          AccountCompany());
	c_setstring("accountCurrency",         AccountCurrency());
	c_setint(   "accountFreeMarginMode",   AccountFreeMarginMode());
	c_setint(   "accountLeverage",         AccountLeverage());
	c_setstring("accountName",             AccountName());
	c_setstring("accountNumber",           AccountNumber());
	c_setstring("accountServer",           AccountServer());
	c_setint(   "accountStopoutLevel",     AccountStopoutLevel());
	c_setint(   "accountStopoutMode",      AccountStopoutMode());
	c_setint(   "accountTradeMode",        AccountInfoInteger(ACCOUNT_TRADE_MODE));
	c_setint(   "accountLimitOrders",      AccountInfoInteger(ACCOUNT_LIMIT_ORDERS));
	c_setstring("symbolName",              symbolName);
	c_setdouble("symbolPoint",             Point);
	c_setint(   "symbolDigits",            Digits);
	c_setint(   "symbolStopLevel",         MarketInfo(symbolName, MODE_STOPLEVEL));
	c_setint(   "symbolLotSize",           MarketInfo(symbolName, MODE_LOTSIZE));
	c_setdouble("symbolTickValue",         MarketInfo(symbolName, MODE_TICKVALUE));
	c_setint(   "symbolTickSize",          MarketInfo(symbolName, MODE_TICKSIZE));
	c_setdouble("symbolSwapLong",          MarketInfo(symbolName, MODE_SWAPLONG));
	c_setdouble("symbolSwapShort",         MarketInfo(symbolName, MODE_SWAPSHORT));
	c_setdouble("symbolMinLot",            MarketInfo(symbolName, MODE_MINLOT));
	c_setdouble("symbolLotStep",           MarketInfo(symbolName, MODE_LOTSTEP));
	c_setdouble("symbolMaxLot",            MarketInfo(symbolName, MODE_MAXLOT));
	c_setint(   "symbolSwapType",          MarketInfo(symbolName, MODE_SWAPTYPE));
	c_setint(   "symbolProfitCalcMode",    MarketInfo(symbolName, MODE_PROFITCALCMODE));
	c_setint(   "symbolMarginCalcMode",    MarketInfo(symbolName, MODE_MARGINCALCMODE));
	c_setdouble("symbolMarginInit",        MarketInfo(symbolName, MODE_MARGININIT));
	c_setdouble("symbolMarginMaintenance", MarketInfo(symbolName, MODE_MARGINMAINTENANCE));
	c_setdouble("symbolMarginHadged",      MarketInfo(symbolName, MODE_MARGINHEDGED));
	c_setdouble("symbolMarginRequired",    MarketInfo(symbolName, MODE_MARGINREQUIRED));
	c_setint(   "symbolFreezeLevel",       MarketInfo(symbolName, MODE_FREEZELEVEL));
	c_setint(   "mqlTradeAllowed",         IsTradeAllowed());//MQLInfoInteger(MQL_TRADE_ALLOWED));
	c_setint(   "mqlSignalAllowed",        MQLInfoInteger(MQL_SIGNALS_ALLOWED));
	c_setint(   "mqlDebug",                MQLInfoInteger(MQL_DEBUG));
	c_setint(   "mqlProfiler",             MQLInfoInteger(MQL_PROFILER));
	c_setint(   "mqlTester",               MQLInfoInteger(MQL_TESTER));
	c_setint(   "mqlOptimization",         MQLInfoInteger(MQL_OPTIMIZATION));
	c_setint(   "mqlVisualMode",           MQLInfoInteger(MQL_VISUAL_MODE));

   c_setstring("SetName",            SetName);
	c_setint(   "StopNewBuy",         StopNewBuy);
	c_setint(   "StopBuy",            StopBuy);
	c_setdouble("BaseBuyLot",         BaseBuyLot);
	c_setint(   "FirstBuyFree",       FirstBuyFree);
	c_setint(   "StopNewSell",        StopNewSell);
	c_setint(   "StopSell",           StopSell);
	c_setdouble("BaseSellLot",        BaseSellLot);
	c_setint(   "FirstSellFree",      FirstSellFree);

	c_setint(   "Step",               Step);
	c_setint(   "FirstTakeProfit",    FirstTakeProfit);
	c_setint(   "TakeProfit",         TakeProfit);
	c_setint(   "StopLoss",           StopLoss);
	c_setint(   "MaxGridLevel",       MaxGridLevel);
	c_setdouble("MaxLot",             MaxLot);
	c_setdouble("PipsMultiplier",     PipsMultiplier);
	c_setint(   "AveragingLevel",     AveragingLevel);
	c_setint(   "AverageAll",         AverageAll);
	c_setint(   "CloseMode",          CloseMode);
	c_setint(   "FreeLvl",            FreeLvl);
	c_setint(   "TimeFrame",          tfs[TimeFrame]);
	c_setint(   "Period1",            Period1);
	c_setdouble("Deviation",          Deviation);
	c_setint(   "MinDev",             MinDev);
	c_setint(   "RollBack",           RollBack);
	c_setint(   "Period2",            Period2);
	c_setint(   "Magic",              magic);
	c_setstring("Comment",            CommentText);
	c_setint(   "AutoMM",             AutoMM);
	c_setint(   "MMEquity",           MMEquity);

	for (int n = 0; n < 64; n++) {
		StringInit(actList[n].comment, 26, 95);
	}
	c_setactions(actList, ArraySize(actList));

	//Запуск постинициализации
	c_postInit();

	int timeframes[9];
	int timeframesSize[9];
	tfCount = c_get_timeframes(timeframes, timeframesSize);
	ArrayResize(tfRates, tfCount);

	Print("-> timeframes count: ", tfCount);
	for (int i = 0; i < tfCount; i++) {
		tfRates[i].timeframe = timeframes[i];
		tfRates[i].length    = timeframesSize[i];
		ArrayResize(tfRates[i].rates, timeframesSize[i]);
		tfRates[i].rates[tfRates[i].length - 1].time = 0;
		Print("-> timeframe[", i, "]: ", 
			timeframes[i], "(", timeframesSize[i], ") -> ", 
			tfRates[i].timeframe, "(", tfRates[i].length, ")");
	}




	Print("============ init dll done =============");
	return (true);
}

void InitLook()
{
	kdpi = c_getdpi()/72.0;
	colors[OP_BUY]  = BuyColor;
	colors[OP_SELL] = SellColor;
	ObjectsDeleteAll();

	info.Init();
	info.SetHeader(" Single v{{ver}}");
	
	//ShowControlPanel(info.x + info.dx + 10, info.y);
	info.DrawHistory();
}

void UpdateOrders() {
	int orders_total = OrdersTotal();
	int _type;
	for (int pos = 0; pos < orders_total; pos++) {
		if (!OrderSelect(pos, SELECT_BY_POS))
			continue; // Обрабатываем только успешные выборки

		if (
			!mqlOptimization &&
			(
				(!AverageAll && OrderMagicNumber() != magic) || // В зависимости от настройки обрабатываем либо все, либо только наши
				(OrderSymbol() != symbolName)                   // Обрабатываем ордера только для текущего символа
			)
		) continue;

		_type = OrderType();
		c_add_order(
			OrderTicket(), 
			_type, 
			OrderLots(), 
			OrderOpenPrice(), 
			OrderTakeProfit(), 
			OrderStopLoss(), 
			OrderProfit() + OrderCommission() + OrderSwap()
		);
	}
}

void UpdateAccountInfo() {
	int ticket = c_updateAccount(AccountBalance(), AccountEquity(), AccountProfit());

	while (ticket) {
		if (OrderSelect(ticket, SELECT_BY_TICKET)) {
			datetime close = OrderCloseTime();
			ticket = c_updateOrder(ticket, OrderOpenTime(), close, close ? OrderProfit() + OrderCommission() + OrderSwap() : 0);
		}
		else {
			ticket = c_updateOrder(ticket, 0, 0, 0);
		}
	}

	lastAccountUpdateTime = TimeGMT();
}