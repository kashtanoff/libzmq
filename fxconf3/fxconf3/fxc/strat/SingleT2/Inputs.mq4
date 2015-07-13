int eaMagic  = 0x00000100;

extern string    SetName = "";

extern string    ___Buy_Parameters = "=== Buy Parameters ===";
extern bool      StopNewBuy   = false;
extern bool      StopBuy      = false;
extern double    BaseBuyLot   = 0.01;
extern bool      FirstBuyFree = false;
extern color     BuyColor     = DodgerBlue;

extern string    ___Sell_Parameters = "=== Sell Parameters ===";
extern bool      StopNewSell   = false;
extern bool      StopSell      = false;
extern double    BaseSellLot   = 0.01;
extern bool      FirstSellFree = false;
extern color     SellColor       = Red;

extern string    ___Common_Parameters = "=== Common Parameters ===";
extern int       Step            = 30;
extern int       FirstTakeProfit = 20;
extern int       TakeProfit      = 45;
extern int       StopLoss        = 1000;
extern int       MaxGridLevel    = 100;
extern double    MaxLot          = 10000.0;
extern double    PipsMultiplier  = 1.05;
extern int       AveragingLevel  = 100;
extern bool      AverageAll      = false;
extern int       CloseMode       = 2;
extern int		 RallyBlockMode  = 0;
extern int       FreeLvl         = 100;
extern ENUM_TIMEFRAMES       TimeFrame       = PERIOD_H1;
extern int       Period1         = 24;
extern double    Deviation       = 2;
extern int       MinDev          = 10;
extern int       RollBack        = 5;
extern int       Period2         = 24;
extern ENUM_TIMEFRAMES       FastTimeFrame   = PERIOD_M15;
extern int	     FastPeriod      = 5;
extern double    FastSpeed       = 4.0;
extern int       MaxPower		 = 5;
extern int       PowerPeriod     = 1;
extern int       Magic           = 0;
extern string    CommentText     = "fxc";
extern int       Slippage        = 3;
//extern string    Skype           = "skype_name";
extern bool      AutoMM          = false;
extern int       MMEquity        = 20000;
extern int       DrawHistoryDays = 14;

void InitParams() {
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
	c_setint(   "RallyBlockMode",     RallyBlockMode);
	c_setint(   "FreeLvl",            FreeLvl);
	c_setint(   "TimeFrame",          tfs[TimeFrame]);
	c_setint(   "Period1",            Period1);
	c_setdouble("Deviation",          Deviation);
	c_setint(   "MinDev",             MinDev);
	c_setint(   "RollBack",           RollBack);
	c_setint(   "Period2",            Period2);
    c_setint(   "FastTimeFrame",      tfs[FastTimeFrame]);
	c_setint(   "FastPeriod",         FastPeriod);
	c_setdouble("FastSpeed",          FastSpeed);
	c_setint(   "MaxPower",           MaxPower);
	c_setint(   "PowerPeriod",        PowerPeriod);
	c_setint(   "Magic",              magic);
	c_setstring("Comment",            CommentText);
	c_setint(   "AutoMM",             AutoMM);
	c_setint(   "MMEquity",           MMEquity);
}