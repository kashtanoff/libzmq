//+------------------------------------------------------------------+
//|                                                          {{ver}}   |
//|                                      Copyright 2013, Aurora ltd. |
//|                                      http://www.fxconfidence.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013, Aurora ltd."
#property link      "http://www.fxconfidence.com"
#property version "{{ver}}"  
#property description "EA FXConfidence"

#include "Inputs.mq4"

#import "fxc{{build}}.dll"
	bool   c_init(int number, string server, string symbol);
	void   c_deinit();
	void   c_postInit();

	void   c_setint(string prop, int value);
	void   c_setdouble(string prop, double value);
	void   c_setvar(string prop, bool& var);
	void   c_setvar(string prop, int& var);
	void   c_setvar(string prop, int& var[]);
	void   c_setvar(string prop, double& var);
	void   c_setvar(string prop, double& var[]);

	void   c_refresh_init(double ask, double bid, double equity);
	int    c_refresh_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0);
	double c_norm_lot(double _lots);
	int    c_getjob();
	int    c_getdpi();
	int    c_get_closed();
	void   c_refresh_prices(double& _closes[], double& _highs[], double& _lows[], int bars);
#import

#include "Errors.mq4"
#include "Order.mq4"
#include "Graph.mq4"
#include "Look.mq4"

//----- Глобальные переменные и определения -----
int tfs[7] = {PERIOD_M1, PERIOD_M5, PERIOD_M15, PERIOD_M30, PERIOD_H1, PERIOD_H4, PERIOD_D1};
int k;      //Коэффициент пересчета старых пунктов в новые
int is_optimization;
int is_testing;
int is_visual;
string symbol;
double lot_step;
double req_margin;
double StartEqu;

//----- Общие с DLL переменные
int    count[2];
double open_dd[2];
double total_lots[2];
int    max_lvl;
double max_dd;
double indicator;
double indicator2;
double prev_ind;
double closes[];
double highs[];
double lows[];
int    buf_len;
bool   run_allowed;
int    current_job;


int    o_ticket;
int    o_type;
double o_lots;
double o_openprice;
double o_tpprice;
double o_slprice;
int    intret;

int    timeout;

bool   showinfo;
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
	is_optimization = IsOptimization();
	is_testing      = IsTesting();
	is_visual       = IsVisualMode();
	symbol          = Symbol();
	lot_step    = MarketInfo(symbol, MODE_LOTSTEP);
	req_margin  = MarketInfo(symbol, MODE_MARGINREQUIRED);
	StartEqu    = AccountEquity();
	showinfo    = (is_optimization || (is_testing && !is_visual)) ? false : true; 
	ECN_Safe    = (is_optimization || is_testing) ? false : true;
	run_allowed = true;

	if (showinfo) InitLook();
	if (!IsDllsAllowed()) CriticalError("DLL is not allowed");
	if (!IsTradeAllowed()) CriticalError("Trade is not allowed");
	if (!DllInit()) return (INIT_FAILED);
	if (SetName != symbol)
	{
		//MessageBox("SetName must be - '" + symbol + "'", "Set name error");
		CriticalError("SetName='" + SetName + "', but must be - '" + symbol + "'");
		//return(INIT_FAILED);
	}
	if (MaxLot < ConfirmLot)
		CriticalError("MaxLot must be higher then ConfirmLot!");
   
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

	if (showinfo) 
		EventSetTimer(1);

	return (INIT_SUCCEEDED);
}

void OnTick()
{
	if (!run_allowed)
		return;

	current_job = -1;
	InitTick(); //Обновляем информацию об ордерах
	while (current_job != 0) //Пока есть работа, выполняем ее
	{
		current_job = c_getjob();
		//Print("Tick: ", ticks, ", Ask: ", Ask, ", Bid: ", Bid); 
		//Print("GetJob: ", r, ", ticket=", o_ticket, ", type=", o_type, ", lots=", o_lots, ", openprice=", o_openprice, ", sl=", o_slprice, ", tp=", o_tpprice);
		switch (current_job)
		{
			case 0: Sleep(timeout); break;  //Больше работы нет, выход
			case 1: Order::Create(); break;
			case 2: Order::Modify(); break;
			case 3: Order::Delete(); break;
			case 4: Order::Close(); break;
		}
	}

	if (!is_optimization)
	{
		if (showinfo) info.Show();

		if (!is_testing)
			return;

		if (LogDD > 0) {
			for (int i = 0; i < 2; i++)
			{
				if (lastlevel[i] > 0 && count[i] == 0 && netdd[i] > LogDD) { //Сброс пирамиды
					FileWrite(herror, "===============-> ", TimeToStr(startdate[i], TIME_DATE), " - ", TimeToStr(TimeCurrent(), TIME_DATE), "   [", netlevel[i], "] - (", netdd[i], ")");
				}
				else if (lastlevel[i] == 0 && count[i] > 0) { //Начало пирамиды
					startdate[i] = TimeCurrent();
					netdd[i]     = 0;
					netlevel[i]  = 0;
				}
				else if (count[i] > 0) {
					netdd[i]    = fmax(netdd[i], fabs(open_dd[i]));
					netlevel[i] = fmax(netlevel[i], count[i]);
				}
				lastlevel[i] = count[i];
			}
		}

		if (lastday != Day()) { //Новый день
			FileWrite(hddlog, TimeToStr(lastdate, TIME_DATE), ";", DoubleToStr(daydd, 2), ";", DoubleToStr(daydd / (AccountEquity()/100.0), 2), "%");
			lastday  = Day();
			lastdate = TimeCurrent();
			daydd    = 0;
		}
		else {
			daydd = fmax(daydd, AccountBalance()-AccountEquity());
		}
	}
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
	if (SaveMap && (is_optimization || is_testing)) //Заполнение карты оптимизации x:TP, y:Step
	{
		string map[100][100];
		string tmp;
		int curTP;
		int curStep = 0;
		string filename = "map " + symbol + ".csv";
		int handle = FileOpen(filename, FILE_READ | FILE_CSV); //Сначала читаем карту

		//Print("FileOpen Error: ", Error(GetLastError()));
		ResetLastError();
		if (handle != INVALID_HANDLE)
		{
			while (!FileIsEnding(handle))
			{
				map[curStep][curTP++] = FileReadString(handle);
				if (FileIsLineEnding(handle))
				{
					curTP = 0;
					curStep++;
				}
			}
			FileClose(handle);
		}

		tmp = "=" + DoubleToStr(fmax(0.0, TesterStatistics(STAT_PROFIT)), 2) + "/" + DoubleToStr(fmax(1, TesterStatistics(STAT_EQUITY_DD)), 2);
		StringReplace(tmp, ".", ",");
		map[Step - 1][TakeProfit - 1] = tmp;

		//Print("------------------| ", tmp);
		//Print("Error: ", Error(GetLastError()));
		
		handle = FileOpen(filename, FILE_WRITE | FILE_CSV); //Теперь пишем обновленную карту в файл
		string accu;
		for (int s = 0; s<100; s++)
		{
			accu = "";
			for (int t = 0; t<100; t++)
			{
				StringAdd(accu, ";" + (StringLen(map[s][t]) < 1 ? " " : map[s][t]));
			}
			FileWrite(handle, StringSubstr(accu, 1));
		}
		FileClose(handle);

		//Print("Error: ", Error(GetLastError()));
	}

	return (TesterStatistics(STAT_PROFIT) / fmax(1, TesterStatistics(STAT_EQUITY_DD)));
	//return(fmax(0.0, AccountEquity()-StartEqu) / fmax(1, max_dd));
}

void OnTimer()
{
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
			curlots = curlots - lot_step - 0.001;
		}
		else if (sparam == "BtnPlus") {
			curlots = curlots + lot_step;
		}
		else if (sparam == "EdtBuyLot") {
			curlots = buylot;
		}
		else if (sparam == "EdtSellLot") {
			curlots = selllot;
		}
		else if (sparam == "BtnBuy") {
			o_type      = OP_BUY;
			o_lots      = c_norm_lot(curlots);
			o_openprice = Ask;
			o_slprice   = Ask - StopLoss * k * Point;
			o_tpprice   = Ask + TakeProfit * k * Point;
			intret      = 100;
			Order::Create();
		}
		else if (sparam == "BtnSell") {
			o_type      = OP_SELL;
			o_lots      = c_norm_lot(curlots);
			o_openprice = Bid;
			o_slprice   = Bid + StopLoss * k * Point;
			o_tpprice   = Bid - TakeProfit * k * Point;
			intret      = 100;
			Order::Create();
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

		if (show_cpanel)
		{
			ObjectSetString(0, "EdtLot", OBJPROP_TEXT, DoubleToStr(o_lots, 2));
			Sleep(100);
			ObjectSetInteger(0, sparam, OBJPROP_STATE, false);
		}
	}
}

//+------------------------------------------------------------------+
//| Прочие функции                                                   |
//+------------------------------------------------------------------+

bool DllInit()
{
	
	run_allowed = c_init(AccountNumber(), AccountServer(), Symbol());
	if (run_allowed) {
		info.Set("handler", "DLL init OK");
		Print("DLL init OK");
	} else {
		reason = "Dll init failed";
		return (false);
	}

	c_setdouble("point",              Point);
	c_setint(   "digits",             Digits);
	c_setdouble("lot_step",           lot_step);
	c_setdouble("lot_min",            MarketInfo(symbol, MODE_MINLOT));
	c_setdouble("lot_max",            MarketInfo(symbol, MODE_MAXLOT));
	c_setdouble("min_sl_tp",          MarketInfo(symbol, MODE_STOPLEVEL) * Point);
	c_setdouble("freeze",             MarketInfo(symbol, MODE_FREEZELEVEL) * Point);
	c_setint(   "is_optimization",    is_optimization);
	c_setint(   "is_visual",          is_visual);
	c_setint(   "is_testing",         is_testing);

	c_setint(   "_stop_new[0]",       StopNewBuy);
	c_setint(   "_stop_new[1]",       StopNewSell);
	c_setint(   "_stop_avr[0]",       StopBuy);
	c_setint(   "_stop_avr[1]",       StopSell);
	c_setint(   "_max_grid_lvl",      MaxGridLevel);
	c_setdouble("_step",              Step * k * Point);
	c_setdouble("_takeprofit",        TakeProfit * k * Point);
	c_setint(   "_forward_lvl",       ForwardLevel);
	c_setint(   "_av_lvl",            AveragingLevel);
	c_setdouble("_av_lot",            RegresAverageLot);
	c_setint(   "_op_av_lvl",         OppositeAverageLevel);
	c_setdouble("_pips_mult",         Pips_Multiplier);
	c_setint(   "_safe_copy",         Safe_Copy);
	c_setdouble("_sell_lot",          BaseSellLot);
	c_setdouble("_buy_lot",           BaseBuyLot);
	c_setdouble("_maxlot",            MaxLot);
	c_setdouble("_lot_hadge_mult",    LotHadgeMult);
	c_setdouble("_regres_mult",       RegresMult);
	c_setint(   "_trend_lvl",         Trend_Level);
	c_setdouble("_trend_lot_mult",    Trend_Lot_Mult);
	c_setdouble("_trend_progress",    Trend_Mult_Progres);
	c_setint(   "_repeat_lvl",        Repeat_Level);
	c_setdouble("_repeat_lot_mult",   Repeat_Lot_Mult);
	c_setdouble("_repeat_progress",   Repeat_Mult_Progres);
	c_setint(   "_period",            PeriodF);
	c_setdouble("_deviation",         Deviation);
	c_setdouble("_stoploss",          StopLoss * k * Point);
	c_setint(   "_attemts",           NumAttemts);
	c_setint(   "_auto_mm",           AutoMM);
	c_setint(   "_mm_equ",            MMEquity);
	c_setdouble("_basket_hadge_mult", BasketHadgeMult);
	c_setdouble("_forward_step_mult", ForwardStepMult);
	c_setdouble("_delta",             Delta);
	c_setint(   "_first_free",        FirstFree);
	c_setint(   "_new_bar",           NewBar);
	c_setint(   "_free_lvl",          FreeLvl);
	c_setdouble("_multf",             MultF);
	c_setint(   "_periodf2",          PeriodF2);
	c_setint(   "_periodf3",          PeriodF3);

	buf_len = 2 * fmax(PeriodF, fmax(PeriodF2, PeriodF3)) + 2;
	ArrayResize(closes, buf_len);
	ArrayResize(highs, buf_len);
	ArrayResize(lows, buf_len);

	c_setint(   "_buf_len",           buf_len - 1);
	c_setdouble("_rollback",          RollBack);
	c_setdouble("_weighthadge",       WeightHadgeMult);
	c_setint(   "_opp_close",         CloseMode);

	c_setvar("open_dd",      open_dd);
	c_setvar("total_lots",   total_lots);
	c_setvar("max_lvl",      max_lvl);
	c_setvar("max_dd",       max_dd);
	c_setvar("indicator",    indicator);
	c_setvar("count_p",      count);
	c_setvar("o_ticket",     o_ticket);
	c_setvar("o_type",       o_type);
	c_setvar("o_lots",       o_lots);
	c_setvar("o_openprice",  o_openprice);
	c_setvar("o_slprice",    o_slprice);
	c_setvar("o_tpprice",    o_tpprice);
	c_setvar("indicator2",   indicator2);
	c_setvar("intret",       intret);

	c_setvar("isRunAllowed", run_allowed);

	//Запуск постинициализации
	c_postInit();
   
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
	info.Set("header",    "ConfidenceEA v{{ver}}");
	info.Set("line",      "----------------------");
   
	if (!run_allowed)
	{
		CriticalError(reason);
		return;
	}

	info.Set("profit",    "Open profit:", 0.0);
	info.Set("buy_lvl",   "  Buy level:", 0);
	info.Set("buy_lots",  "       lots:", 0.0);
	info.Set("sell_lvl",  " Sell level:", 0);
	info.Set("sell_lots", "       lots:", 0.0);
	info.Set("balance",   "    Balance:", 0.0);
	
	if (is_visual)
	{
		info.Set("dd",      "     Max DD:", 0.0);
		info.Set("max_lvl", "  Max level:", 0);
		info.Set("ind1",    "Indicator 1:", 0.0);
		info.Set("ind2",    "Indicator 2:", 0.0);
		info.Set("lot",     "Lot size:", MarketInfo(Symbol(), MODE_LOTSIZE));
		info.Set("spread",  "Spread:",   MarketInfo(Symbol(), MODE_SPREAD));
		info.Set("minlot",  "Min lot:",  MarketInfo(Symbol(), MODE_MINLOT));
	}
	
	//ShowControlPanel(info.x + info.dx + 10, info.y);
	info.DrawHistory();
}

void InitTick() {
	if (lastbar != Time[1]) {
		if (
			(CopyClose(symbol, tfs[TimeFrame], 0, buf_len, closes) >= buf_len) &&
			(CopyHigh(symbol, tfs[TimeFrame], 0, buf_len, highs) >= buf_len) &&
			(CopyLow(symbol, tfs[TimeFrame], 0, buf_len, lows) >= buf_len)
		) {
			c_refresh_prices(closes, highs, lows, iBars(symbol, tfs[TimeFrame]));
		} else {
			Print("Error copy rates, buf_len=", buf_len);
		}
	}

	lastbar = Time[1];
	c_refresh_init(Ask, Bid, AccountEquity());

	int orders_total = OrdersTotal();
	int _type;
	for (int pos = 0; pos < orders_total; pos++) {
		if (!OrderSelect(pos, SELECT_BY_POS))
			continue; // Обрабатываем только успешные выборки

		if (
			!is_optimization &&
			(
				(!AverageAll && OrderMagicNumber() != Magic) || // В зависимости от настройки обрабатываем либо все, либо только наши
				(OrderSymbol() != symbol)                       // Обрабатываем ордера только для текущего символа
			)
		) continue;

		_type = OrderType();
		c_refresh_order(
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