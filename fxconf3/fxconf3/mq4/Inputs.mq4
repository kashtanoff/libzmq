extern string    SetName = "";

extern string    ___Manual_Control = "=== Manual Control ===";
extern bool      StopNewBuy   = false;
extern bool      StopNewSell  = false;
extern bool      StopBuy      = false;
extern bool      StopSell     = false;
extern int       MaxGridLevel = 100;

extern string    ___Grid_Parameters = "=== Grid Parameters ===";
extern int       Step            = 30;
extern int       TakeProfit      = 45;
extern int       ForwardLevel    = 100;
extern double    ForwardStepMult = 1.0;

extern string    ___Averaging_Parameters = "=== Averaging Parameters ===";
extern int       AveragingLevel       = 100;
extern double    RegresAverageLot     = 10000;
extern bool      AverageAll           = false;
extern int       OppositeAverageLevel = 100;

extern string    ___Lot_Control = "=== Lot Control ===";
extern double    Pips_Multiplier = 1.0;
extern bool      Safe_Copy       = false;
extern double    BaseSellLot     = 0.01;
extern double    BaseBuyLot      = 0.01;
extern double    MaxLot          = 10000.0;
extern double    ConfirmLot      = 5;
extern double    LotHadgeMult    = 0.0;
extern double    BasketHadgeMult = 0.0;
extern double    WeightHadgeMult = 0.0;
extern double    RegresMult      = 0.0;

extern string    ___Trend_and_Repeat_Multipiers = "=== Trend & Repeat Multipliers ===";
extern int       Trend_Level         = 100;
extern double    Trend_Lot_Mult      = 1.0;
extern double    Trend_Mult_Progres  = 1.0;
extern int       Repeat_Level        = 100;
extern double    Repeat_Lot_Mult     = 1.0;
extern double    Repeat_Mult_Progres = 1.0;

extern string    ___Filter = "=== Base lot multiplier by RSI ===";
extern int       CloseMode = 2;
extern bool      NewBar    = false;
extern bool      FirstFree = false;
extern int       PeriodF   = 24;
extern int       FreeLvl   = 100;
extern double    Deviation = 2;
extern double    Delta     = 0.001;
extern double    RollBack  = 0.0005;
//extern double    DeltaF2   = 30;
//extern double    DeltaF3   = 2;
extern int       TimeFrame = 4;
extern int       Shift     = 1;
extern int       Switch    = 1;
extern double    MultF     = 250;
extern int       PeriodF2  = 24;
extern int       PeriodF3  = 0;

extern string    ___Common_Parameters = "=== Common Parameters ===";
extern int       StopLoss        = 5000;
extern int       Magic           = 88555;
extern string    CommentText     = "fxc";
extern color     BuyColor        = DodgerBlue;
extern color     SellColor       = Red;
extern int       Slippage        = 3;
extern int       NumAttemts      = 10;
extern string    Skype           = "skype_name";
extern bool      AutoMM          = false;
extern int       MMEquity        = 20000;
extern int       DrawHistoryDays = 14;
extern bool      SaveMap         = false;
extern bool      ShowDebug       = false;
extern double    LogDD           = 0;
extern bool      ECN_Safe        = true;