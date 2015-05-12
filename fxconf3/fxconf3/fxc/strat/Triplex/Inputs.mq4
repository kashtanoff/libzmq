//OC Triplex
int eaMagic  = 0x00000300;

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

extern string    ___Channel1 = "=== Channel 1 ===";
extern int		 Step1			 = 30;
extern int       TakeProfit1     = 45;
extern int       MaxLevel1       = 1;
extern int       Period1         = 30;
extern double    Deviation1      = 1.2;
extern int       DevPeriod1      = 50;
extern ENUM_TIMEFRAMES       Timeframe1      = PERIOD_M15;

extern string    ___Channel2 = "=== Channel 2 ===";
extern int		 Step2			 = 30;
extern int       TakeProfit2     = 45;
extern int       MaxLevel2       = 2;
extern int       Period2         = 30;
extern double    Deviation2      = 1.2;
extern int       DevPeriod2      = 50;
extern ENUM_TIMEFRAMES       Timeframe2      = PERIOD_H1;

extern string    ___Channel3 = "=== Channel 3 ===";
extern int		 Step3			 = 30;
extern int       TakeProfit3     = 45;
extern int       MaxLevel3       = 100;
extern int       Period3         = 30;
extern double    Deviation3      = 1.2;
extern int       DevPeriod3      = 50;
extern ENUM_TIMEFRAMES       Timeframe3      = PERIOD_H4;

extern string    ___Common_Parameters = "=== Common Parameters ===";
extern int       StopLoss        = 1000;
extern double    MaxLot          = 10000.0;
extern double    PipsMultiplier  = 1.1;
extern int       AveragingLevel  = 100;
extern bool      AverageAll      = false;
extern int       CloseMode       = 0;
extern int       FreeLvl         = 100;
extern int       MinDev          = 10;
extern int       RollBack        = 5;
extern int       Magic           = 0;
extern string    CommentText     = "oct";
extern int       Slippage        = 3;
//extern string    Skype           = "skype_name";
extern bool      AutoMM          = false;
extern int       MMEquity        = 20000;
extern int       DrawHistoryDays = 14;
extern int		 TestMode		 = 0;

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

	c_setint(   "Step1",              Step1);
	c_setint(   "TakeProfit1",        TakeProfit1);
	c_setint(   "MaxLevel1",          MaxLevel1);
	c_setint(   "Period1",            Period1);
	c_setdouble("Deviation1",         Deviation1);
	c_setint(   "DevPeriod1",         DevPeriod1);
	c_setint(   "Timeframe1",         Timeframe1);

	c_setint(   "Step2",              Step2);
	c_setint(   "TakeProfit2",        TakeProfit2);
	c_setint(   "MaxLevel2",          MaxLevel2);
	c_setint(   "Period2",            Period2);
	c_setdouble("Deviation2",         Deviation2);
	c_setint(   "DevPeriod2",         DevPeriod2);
	c_setint(   "Timeframe2",         Timeframe2);

	c_setint(   "Step3",              Step3);
	c_setint(   "TakeProfit3",        TakeProfit3);
	c_setint(   "MaxLevel3",          MaxLevel3);
	c_setint(   "Period3",            Period3);
	c_setdouble("Deviation3",         Deviation3);
	c_setint(   "DevPeriod3",         DevPeriod3);
	c_setint(   "Timeframe3",         Timeframe3);


	c_setint(   "StopLoss",           StopLoss);
	c_setdouble("MaxLot",             MaxLot);
	c_setdouble("PipsMultiplier",     PipsMultiplier);
	c_setint(   "AveragingLevel",     AveragingLevel);
	c_setint(   "AverageAll",         AverageAll);
	c_setint(   "CloseMode",          CloseMode);
	c_setint(   "FreeLvl",            FreeLvl);
	c_setint(   "MinDev",             MinDev);
	c_setint(   "RollBack",           RollBack);
	c_setint(   "Magic",              magic);
	c_setstring("Comment",            CommentText);
	c_setint(   "AutoMM",             AutoMM);
	c_setint(   "MMEquity",           MMEquity);
	c_setint(   "TestMode",           TestMode);
}