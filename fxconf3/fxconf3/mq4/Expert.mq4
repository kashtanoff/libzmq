//+------------------------------------------------------------------+
//|                                                          {{ver}}   |
//|                                      Copyright 2013, Aurora ltd. |
//|                                      http://www.fxconfidence.com |
//+------------------------------------------------------------------+
#property copyright "Copyright 2013, Aurora ltd."
#property link      "http://www.fxconfidence.com"
#property version "{{ver}}"  
#property description "EA FXConfidence"

//----- Входные параметры -----
extern string    SetName            = "";
extern string    ___Manual_Control  = "=== Manual Control ===";
extern bool      StopNewBuy         = false;
extern bool      StopNewSell        = false;
extern bool      StopBuy            = false;
extern bool      StopSell           = false;
extern int       MaxGridLevel       = 100;

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
extern int       Trend_Level        = 100;
extern double    Trend_Lot_Mult     = 1.0;
extern double    Trend_Mult_Progres = 1.0;
extern int       Repeat_Level       = 100;
extern double    Repeat_Lot_Mult    = 1.0;
extern double    Repeat_Mult_Progres= 1.0;

extern string    ___Filter = "=== Base lot multiplier by RSI ===";
extern int       CloseMode = 2;
extern bool      NewBar = false;
extern bool      FirstFree = false;
extern int       PeriodF   = 24;
extern int       FreeLvl   = 100;
extern double    Deviation = 2;
extern double    Delta   = 0.001;
extern double    RollBack = 0.0005;
//extern double    DeltaF2   = 30;
//extern double    DeltaF3   = 2;
extern int       TimeFrame = 4;
extern int       Shift = 1;
extern int       Switch = 1;
extern double    MultF = 250;
extern int       PeriodF2 = 24;
extern int       PeriodF3 = 0;

extern string    ___Common_Parameters = "=== Common Parameters ===";
extern int       StopLoss     = 5000;
extern int       Magic        = 88555;
extern string    CommentText  = "fxc";
extern color     BuyColor     = DodgerBlue;
extern color     SellColor    = Red;
extern int       Slippage     = 3;
extern int       NumAttemts   = 10;
extern string    Skype        = "skype_name";
extern bool      AutoMM       = false;
extern int       MMEquity     = 20000;
extern int       DrawHistoryDays = 14;
extern bool      SaveMap      = false;
extern bool      ShowDebug    = false; 
extern double    LogDD        = 0;
extern bool      ECN_Safe     = true;

//---------------------------- Основная библиотека C++ ---------------------------
#import "fxc{{build}}.dll"
int    c_init(string symbol);
void   c_setint(int handler, int index, int value);
void   c_setdouble(int handler, int index, double value);
void   c_setvar(int handler, int index, int& var);
void   c_setvar(int handler, int index, double& var);
void   c_setvar(int handler, int index, int& var[]);
void   c_setvar(int handler, int index, double& var[]);
void   c_refresh_init(int handler, double ask, double bid, double equity);
int    c_refresh_order(int handler, int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit=0);
double c_norm_lot(int handler, double _lots);
int    c_getjob(int handler);
int    c_getdpi();
int    c_get_closed(int handler);
void   c_deinit(int handler);
void   c_refresh_prices(int handler, double& _closes[], double& _highs[], double& _lows[], int bars);
#import
//----- Глобальные переменные и определения -----
int tfs[7] = {PERIOD_M1, PERIOD_M5, PERIOD_M15, PERIOD_M30, PERIOD_H1, PERIOD_H4, PERIOD_D1};
int h;      //Хандлер окна для dll
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
bool   run;


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
bool   global_stop = false;
string reason = "";
datetime lastbar;
datetime startdate[2];
int netlevel[2];
datetime enddate[2];
int lastlevel[2]= {0, 0};
double netdd[2] = {0, 0};
//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
int OnInit()
{
   k = (Digits == 3 || Digits == 5)? 10: 1;  //Приведение параметров к дельте цены с переводом в старые пункты
   is_optimization = IsOptimization();
   is_testing = IsTesting();
   is_visual = IsVisualMode();
   symbol = Symbol();
   lot_step = MarketInfo(symbol, MODE_LOTSTEP);
   req_margin = MarketInfo(symbol, MODE_MARGINREQUIRED);
   StartEqu = AccountEquity();
   showinfo = (is_optimization || (is_testing && !is_visual))? false: true; 
   ECN_Safe = (is_optimization || is_testing)? false: true;
   run_allowed = true;
   if (showinfo) InitLook();
   if (!IsDllsAllowed()) CriticalError("DLL not allowed");
   if (!IsTradeAllowed()) CriticalError("Trade not allowed");
   if (!DllInit()) return(INIT_FAILED);
   
   if (SetName != symbol)
   {
      //MessageBox("SetName must be - '" + symbol + "'", "Set name error");
      CriticalError("SetName='" + SetName + "', but must be - '" + symbol + "'");
      //return(INIT_FAILED);
   }
   if (MaxLot < ConfirmLot)
      CriticalError("MaxLot must be higher then ConfirmLot!");
   
   herror = FileOpen("errors.txt", FILE_WRITE|FILE_TXT);
   FileSeek(herror, 0, SEEK_END);
   FileWrite(herror, "======= Start =======");
   max_dd = 0;
   max_lvl = 0;
   
   //Print("Init Done");
   if (showinfo) EventSetTimer(1);
   hddlog = FileOpen("dd_log.txt", FILE_WRITE|FILE_TXT);
   FileSeek(hddlog, 0, SEEK_END);
   daydd = 0;
   lastday = Day();
   lastdate = TimeCurrent();
   return(INIT_SUCCEEDED);
}
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+
void OnTick()
{
   if (!run_allowed)
      return;
   run = true;
   //if (global_stop) return;
   if (Year()>2014)
   {
      global_stop = true;
      CriticalError("Free Period Expiried");
   }
   int r;
   //ticks++;
   Refresh();   //Обновляем информацию об ордерах
   while (run)   //Пока есть работа, выполняем ее
   {
      r = c_getjob(h);
      //Print("Tick: ", ticks, ", Ask: ", Ask, ", Bid: ", Bid); 
      //Print("GetJob: ", r, ", ticket=", o_ticket, ", type=", o_type, ", lots=", o_lots, ", openprice=", o_openprice, ", sl=", o_slprice, ", tp=", o_tpprice);
      switch (r)
      {
         case 0: Sleep(timeout); run = false; break;  //Больше работы нет, выход
         case 1: NewOrder(); break;
         case 2: ModOrder(); break;
         case 3: DelOrder(); break;
         case 4: CloseOrder(); break;
      }
   }
   
   if (!is_optimization)
   {
      if (showinfo) ShowInfo();
      if (LogDD > 0)
         for (int i=0; i<2; i++)
         {
            if (lastlevel[i] > 0 && count[i] == 0 && netdd[i] > LogDD)  //Сброс пирамиды
            {
               FileWrite(herror, "===============-> ", TimeToStr(startdate[i], TIME_DATE), " - ", TimeToStr(TimeCurrent(), TIME_DATE), "   [", netlevel[i], "] - (", netdd[i], ")");
            }
            else if (lastlevel[i] == 0 && count[i] > 0)  //Начало пирамиды
            {
               startdate[i] = TimeCurrent();
               netdd[i] = 0;
               netlevel[i] = 0;
            }
            else if (count[i] > 0)
            {
               netdd[i] = fmax(netdd[i], fabs(open_dd[i]));
               netlevel[i] = fmax(netlevel[i], count[i]);
            }
            lastlevel[i] = count[i];
         }
      if (lastday != Day())  //Новый день
      {
         FileWrite(hddlog, TimeToStr(lastdate, TIME_DATE), ";", DoubleToStr(daydd, 2), ";", DoubleToStr(daydd / (AccountEquity()/100.0), 2), "%");
         lastday = Day();
         lastdate = TimeCurrent();
         daydd = 0;
      }
      else
      {
         daydd = fmax(daydd, AccountBalance()-AccountEquity());
      }
   }
}
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int r)
{
   ObjectsDeleteAll();
   Print("Deinit, ", ticks, " ticks done");
   c_deinit(h);
   FileClose(herror);
   FileClose(hddlog);
}
//+------------------------------------------------------------------+
//| Прочие функции                                                   |
//+------------------------------------------------------------------+
int DllInit()
{
   h = c_init(Symbol());
   
   if (h<0)
   {
      reason = "dll not init";
      run_allowed = false;
      return(0);
   }
   else
   {
      info.Set("handler", "DLL init OK", h, 0);
      Print("DLL init OK");
      }
   c_setdouble(h,  1, Point);
   c_setint(h,     2, Digits);
   c_setdouble(h,  4, lot_step);
   c_setdouble(h,  5, MarketInfo(symbol, MODE_MINLOT));
   c_setdouble(h,  6, MarketInfo(symbol, MODE_MAXLOT));
   c_setdouble(h,  7, MarketInfo(symbol, MODE_STOPLEVEL) * Point);
   c_setdouble(h,  8, MarketInfo(symbol, MODE_FREEZELEVEL) * Point);
   c_setint(h,     9, is_optimization);
   c_setint(h,    10, is_visual);
   c_setint(h,    11, is_testing);
  
   c_setint(h,    50, StopNewBuy);
   c_setint(h,    51, StopNewSell);
   c_setint(h,    52, StopBuy);
   c_setint(h,    53, StopSell);
   c_setint(h,    54, MaxGridLevel);
   c_setdouble(h, 55, Step * k * Point);
   c_setdouble(h, 57, TakeProfit * k * Point);  
   c_setint(h,    59, ForwardLevel);
   c_setint(h,    64, AveragingLevel);
   c_setdouble(h, 65, RegresAverageLot);
   c_setint(h,    66, OppositeAverageLevel);
   c_setdouble(h, 67, Pips_Multiplier);
   c_setint(h,    68, Safe_Copy);
   c_setdouble(h, 69, BaseSellLot);
   c_setdouble(h, 97, BaseBuyLot);
   c_setdouble(h, 70, MaxLot);
   c_setdouble(h, 71, LotHadgeMult);
   c_setdouble(h, 72, RegresMult);
   c_setint(h,    73, Trend_Level);
   c_setdouble(h, 74, Trend_Lot_Mult);
   c_setdouble(h, 75, Trend_Mult_Progres);
   c_setint(h,    76, Repeat_Level);
   c_setdouble(h, 77, Repeat_Lot_Mult);
   c_setdouble(h, 78, Repeat_Mult_Progres);
   c_setint(h,    79, PeriodF);
   c_setdouble(h, 80, Deviation);
   c_setdouble(h, 81, StopLoss * k * Point);
   c_setint(h,    82, NumAttemts);
   c_setint(h,    83, AutoMM);
   c_setint(h,    84, MMEquity);
   c_setdouble(h, 85, BasketHadgeMult);
   c_setdouble(h, 86, ForwardStepMult);
   c_setdouble(h, 87, Delta);
   c_setint(h,    88, FirstFree);
   c_setint(h,    89, NewBar);
   c_setint(h,    90, FreeLvl);
   c_setdouble(h, 91, MultF);
   c_setint(h,    92, PeriodF2);
   c_setint(h,    93, PeriodF3);
   buf_len = 2*fmax(PeriodF, fmax(PeriodF2, PeriodF3))+2;
   ArrayResize(closes, buf_len);
   ArrayResize(highs, buf_len);
   ArrayResize(lows, buf_len);
   c_setint(h,    94, buf_len-1);
   c_setdouble(h, 95, RollBack);
   c_setdouble(h, 96, WeightHadgeMult);
   c_setint(h,    97, CloseMode);
   
   c_setvar(h, 102, open_dd);
   c_setvar(h, 103, total_lots);
   c_setvar(h, 104, max_lvl);
   c_setvar(h, 105, max_dd);
   c_setvar(h, 106, indicator);
   c_setvar(h, 107, count);
   c_setvar(h, 110, o_ticket);
   c_setvar(h, 111, o_type);
   c_setvar(h, 112, o_lots);
   c_setvar(h, 113, o_openprice);
   c_setvar(h, 114, o_slprice);
   c_setvar(h, 115, o_tpprice);
   c_setvar(h, 116, indicator2);
   c_setvar(h, 200, intret);
   
   c_setdouble(h, 555, 0);  //Запуск постинициализации
   
   Print("============ init dll done =============");
   return(1);
}
void InitLook()
{
   kdpi = c_getdpi()/72.0;
   colors[OP_BUY] = BuyColor;
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
      info.Set("dd",        "     Max DD:", 0.0);
      info.Set("max_lvl",   "  Max level:", 0);
      info.Set("ind1", "Indicator 1:", 0.0);
      info.Set("ind2", "Indicator 2:", 0.0);
      info.Set("lot", "Lot size:", MarketInfo(Symbol(), MODE_LOTSIZE));
      info.Set("spread", "Spread:", MarketInfo(Symbol(), MODE_SPREAD));
      info.Set("minlot", "Min lot:", MarketInfo(Symbol(), MODE_MINLOT));
   }
   //ShowControlPanel(info.x + info.dx + 10, info.y);
   DrawHistory();
}
void Refresh()
{
   if (lastbar != Time[1])
   {
      if ((CopyClose(symbol, tfs[TimeFrame], 0, buf_len, closes) >= buf_len) &&
          (CopyHigh(symbol, tfs[TimeFrame], 0, buf_len, highs) >= buf_len)&&
          (CopyLow(symbol, tfs[TimeFrame], 0, buf_len, lows) >= buf_len))
          c_refresh_prices(h, closes, highs, lows, iBars(symbol, tfs[TimeFrame]));
      else
         Print("Error copy rates, buf_len=", buf_len);
   }
   lastbar = Time[1];
   c_refresh_init(h, Ask, Bid, AccountEquity());
   int orders_total = OrdersTotal();
   int _type;
   for (int pos=0; pos<orders_total; pos++)
   {
      if (!OrderSelect(pos, SELECT_BY_POS)) continue;   //Обрабатываем только успешные выборки
      if (!is_optimization &&
          ((!AverageAll && OrderMagicNumber() != Magic) ||  //В зависимости от настройки обрабатываем либо все либо только наши
          (OrderSymbol() != symbol)))                       //Обрабатываем ордера только для текущего символа
         continue;
      _type = OrderType();
      c_refresh_order(h, OrderTicket(), _type, OrderLots(), OrderOpenPrice(), OrderTakeProfit(), OrderStopLoss(), OrderProfit() + OrderCommission() + OrderSwap());
                        //(_type < 2 && showinfo)? OrderProfit() + OrderCommission() + OrderSwap(): 0);
   }
}
//========================================== Работа с графикой =======================================
void ShowInfo()
{
   max_dd=fmin(max_dd, AccountEquity() - AccountBalance());
   info.Change("profit", open_dd[OP_BUY] + open_dd[OP_SELL]);
   info.Change("buy_lvl", count[OP_BUY]);
   info.Change("buy_lots", total_lots[OP_BUY]);
   info.Change("sell_lvl", count[OP_SELL]);
   info.Change("sell_lots", total_lots[OP_SELL]);
   info.Change("balance", AccountBalance());
   info.Change("ind1", indicator);
   info.Change("ind2", indicator2);
   if (is_visual)
   {
      info.Change("dd", max_dd);
      info.Change("max_lvl", max_lvl);
   }
   /*double curlot;
   for (int t=OP_BUY; t==OP_SELL; t++)
   {
      curlot = c_normlot((takeprofit * MinLot - c_basket_weight(t, c_tp(t, mpo[t], takeprofit), av_lvl[t])) / takeprofit);
      ObjectSetString(0, lotnames[t], OBJPROP_TEXT, dts(curlot, 2));
   }
   */
   int _ticket;
   bool _flag = false;
   while (true)
   {
      _ticket = c_get_closed(h);
      if (_ticket==0)
         break;
      if (!OrderSelect(_ticket, SELECT_BY_TICKET, MODE_HISTORY))
         continue;
      
      DrawOrder(_ticket, OrderType(), OrderOpenTime(), OrderOpenPrice(), OrderCloseTime(), OrderClosePrice());
      _flag = true;
   }
   if (_flag)
      DelOldDraws();
}
void CriticalError(string msg)
{
   run_allowed = false;
   if (!showinfo) return;
   info.Set("error", "Advisor stopped");
   info.Set("reason", msg);
}
void OnChartEvent(const int id, const long& lparam, const double& dparam, const string& sparam)
{
   if (id == CHARTEVENT_OBJECT_CLICK)
   {
      double buylot = BaseBuyLot;
      double selllot = BaseSellLot;
      double curlots = fmin(BaseBuyLot, BaseSellLot);
      if (show_cpanel)
      {
         curlots = StrToDouble(ObjectGetString(0, "EdtLot", OBJPROP_TEXT));
         buylot = StrToDouble(ObjectGetString(0, "EdtBuyLot", OBJPROP_TEXT));
         selllot = StrToDouble(ObjectGetString(0, "EdtSellLot", OBJPROP_TEXT));
      }
      if (sparam == "BtnMinus")
         curlots = curlots - lot_step - 0.001;
      if (sparam == "BtnPlus")
         curlots = curlots + lot_step;
      if (sparam == "EdtBuyLot")
         curlots = buylot;
      if (sparam == "EdtSellLot")
         curlots = selllot;
      if (sparam == "BtnBuy")
      {
         o_type = OP_BUY;
         o_lots = c_norm_lot(h, curlots);
         o_openprice = Ask;
         o_slprice = Ask - StopLoss * k * Point;
         o_tpprice = Ask + TakeProfit * k * Point;
         intret = 100;
         NewOrder();
      }
      if (sparam == "BtnSell")
      {
         o_type = OP_SELL;
         o_lots = c_norm_lot(h, curlots);
         o_openprice = Bid;
         o_slprice = Bid + StopLoss * k * Point;
         o_tpprice = Bid - TakeProfit * k * Point;
         intret = 100;
         NewOrder();
      }
      if (sparam == "BtnClsB")
      {
         //CloseAllProfit(OP_BUY);
      }
      if (sparam == "BtnClsS")
      {
         //CloseAllProfit(OP_SELL);
      }
      if (sparam == "textbox")
      {
         if (show_cpanel)
            DelControlPanel();
         else
            ShowControlPanel(info.x + info.dx + 10, info.y);
      }
      if (show_cpanel)
      {
      ObjectSetString(0, "EdtLot", OBJPROP_TEXT, DoubleToStr(o_lots, 2));
      Sleep(100);
      ObjectSetInteger(0, sparam, OBJPROP_STATE, false);
      }
   }
}
void OnTimer()
{  
}
void DrawHistory()
{
   int ord_total = OrdersHistoryTotal();
   datetime cur_date = TimeCurrent();
   int seconds = 86400 * DrawHistoryDays;
   datetime close_date;
   datetime open_date;
   double open_price, close_price;
   int ticket;
   int _type;
   for (int i=ord_total; i>=0; i--)
   {
      if (!OrderSelect(i, SELECT_BY_POS, MODE_HISTORY))
         continue;
      close_date = OrderCloseTime();
      _type = OrderType();
      
      if (cur_date - close_date <= seconds && _type <= OP_SELL && OrderSymbol() == symbol)
      {
         open_date   = OrderOpenTime();
         open_price  = OrderOpenPrice();
         close_price = OrderClosePrice();
         ticket = OrderTicket();
         DrawOrder(ticket, _type, open_date, open_price, close_date, close_price);
      }
   }
}
void DrawOrder(int _ticket, int _type, datetime _open_date, double _open_price, datetime _close_date, double _close_price)
{
   if (_close_price == 0)
      return;
   string open_name  = DoubleToStr(_ticket, 0) + " o";
   string close_name = DoubleToStr(_ticket, 0) + " c";
   string line_name  = DoubleToStr(_ticket, 0) + " l";
   // ---- создаём открывающую стрелку ------------
   ObjectCreate(open_name,OBJ_ARROW,0,_open_date,_open_price);//открывающая стрелка Бай-ордера
   ObjectSet(open_name,OBJPROP_ARROWCODE,1);//код стрелки 232 
   ObjectSet(open_name,OBJPROP_COLOR,colors[_type]);//цвет стрелки
   // ---- создаём закрывающую стрелку ------------
   ObjectCreate(close_name,OBJ_ARROW,0,_close_date,_close_price);//закрывающая стрелка Бай-ордера
   ObjectSet(close_name,OBJPROP_ARROWCODE,3);//код стрелки  231
   ObjectSet(close_name,OBJPROP_COLOR,colors[_type]);//цвет стрелки
   // ---- создаём линии ------------ 
   ObjectCreate(line_name,OBJ_TREND,0,_open_date,_open_price,_close_date,_close_price);//
   ObjectSet(line_name,OBJPROP_RAY,false);//запрещаем рисовать луч
   ObjectSet(line_name,OBJPROP_WIDTH,0);//устанавливаем толщину линии ()
   ObjectSet(line_name,OBJPROP_STYLE,0);//устанавливаем тип линии (отрезки)
   ObjectSet(line_name,OBJPROP_STYLE,STYLE_DOT);//устанавливаем тип штриховки(пунктирная линия)
   ObjectSet(line_name,OBJPROP_COLOR,colors[_type]);//устанавливаем цвет(синий/красный)
}
void DelOldDraws()
{
   string _name;
   int _type;
   for (int i=0; i<ObjectsTotal(); i++)
   {
      _name = ObjectName(i);
      _type = ObjectType(_name);
      if (_type != OBJ_ARROW && _type != OBJ_TREND)
         continue;
      if (ObjectGetInteger(0, _name, OBJPROP_TIME) < (TimeCurrent()- 86400 * DrawHistoryDays))
         ObjectDelete(_name);
   }
}
void ShowControlPanel(int x, int y)
{
   Edit(DoubleToStr(BaseSellLot, 2), "EdtSellLot", x, y, 80, 32, clrLightPink, true);
   lotnames[OP_SELL] = "EdtSellLot";
   Edit(DoubleToStr(BaseBuyLot, 2), "EdtBuyLot", x+85, y, 80, 32, clrLightBlue, true);
   lotnames[OP_BUY] = "EdtBuyLot";
   Button("<", "BtnMinus", x, y+37, 35, 32, clrGray);
   Edit(DoubleToStr(fmin(BaseBuyLot, BaseSellLot), 2), "EdtLot", x+40, y+37, 85, 32, clrLightGray);
   Button(">", "BtnPlus", x+130, y+37, 35, 32, clrGray);
   Button("Sell", "BtnSell", x, y+74, 80, 32, clrRed);
   Button("Buy", "BtnBuy", x+85, y+74, 80, 32, clrBlue);
   Button("Close", "BtnClsS", x, y+111, 80, 32, clrRed);
   Button("Close", "BtnClsB", x+85, y+111, 80, 32, clrBlue);
   show_cpanel = true;
}
void DelControlPanel()
{
   ObjectDelete("EdtSellLot");
   ObjectDelete("EdtBuyLot");
   ObjectDelete("EdtLot");
   ObjectDelete("BtnMinus");
   ObjectDelete("BtnPlus");
   ObjectDelete("BtnSell");
   ObjectDelete("BtnBuy");
   ObjectDelete("BtnClsS");
   ObjectDelete("BtnClsB");
   show_cpanel = false;
}
void MakeRect(int x, int y, int dx, int dy, color fill, color border, string name="box")
{
   ObjectCreate(name, OBJ_RECTANGLE_LABEL, 0, 0, 0);
   ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x);
   ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(0, name, OBJPROP_XSIZE, dx);
   ObjectSetInteger(0, name, OBJPROP_YSIZE, dy);
   ObjectSetInteger(0, name, OBJPROP_BGCOLOR, fill);
   ObjectSetInteger(0, name, OBJPROP_BORDER_TYPE, BORDER_FLAT);
   ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
   ObjectSetInteger(0, name, OBJPROP_COLOR, border);
   ObjectSetInteger(0, name, OBJPROP_STYLE, STYLE_SOLID);
   ObjectSetInteger(0, name, OBJPROP_WIDTH, 1);
   ObjectSetInteger(0, name, OBJPROP_SELECTABLE, false);
   ObjectSetInteger(0, name, OBJPROP_SELECTED, false);
   ObjectSetInteger(0, name, OBJPROP_HIDDEN, true);
   ObjectSet(name, OBJPROP_ZORDER, 1);
}
void MakeLabel(string name, int x, int y, string text, int font_size=10, color text_color=clrRed, string _fontname="Consolas")
{
   ObjectCreate(name, OBJ_LABEL, 0, 0, 0);
   ObjectSetInteger(0, name, OBJPROP_XDISTANCE, x);
   ObjectSetInteger(0, name, OBJPROP_YDISTANCE, y);
   ObjectSetInteger(0, name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
   ObjectSetString(0, name, OBJPROP_TEXT, text);
   ObjectSetString(0, name, OBJPROP_FONT, _fontname);
   ObjectSetInteger(0, name, OBJPROP_FONTSIZE, font_size);
   ObjectSetInteger(0, name, OBJPROP_ANCHOR, ANCHOR_LEFT_UPPER);
   ObjectSetInteger(0, name, OBJPROP_COLOR, text_color);
   ObjectSetInteger(0, name, OBJPROP_BACK, false);
   ObjectSetInteger(0, name,OBJPROP_SELECTABLE, false);
   ObjectSetInteger(0, name,OBJPROP_SELECTED, false);
   ObjectSetInteger(0, name, OBJPROP_HIDDEN, false);
   ObjectSetInteger(0, name, OBJPROP_XSIZE, 10);
   WindowRedraw();
}

void ChangeLabel(string name, string text)
{
   ObjectSetString(0, name, OBJPROP_TEXT, text);
}

class Look
{
public:
   string names[20];
   int count;
   int width;
   string labels[20];
   int digits[16];
   int x, y, x2, y2;
   int font_size;
   color text_color;
   int sdx, sdy;
   int dx, dy;
   int spaces;
   string fontname;
   
   void Look(int _font_size, color _text_color, int _x, int _y, string _fontname="Lucida Console")
   {
      font_size = _font_size;
      text_color = _text_color;
      fontname = _fontname;
      GetSymbolSize();
      x = _x;
      y = _y;
      Init();
   }
   void Init()
   {
      count = 0;
      width = 0;
      dx = width*sdx + 10;
      dy = count*sdy + 10;
      MakeRect(x, y, dx, dy, clrDarkBlue, clrBlue, "textbox");
   }
   void GetSymbolSize()
   {
      kdpi = c_getdpi()/72.0;
      TextSetFont(fontname, font_size);
      TextGetSize("-", sdx, sdy); 
      sdy = (int)round(sdy * kdpi);
      sdx = (int)round(sdx * kdpi);
   }
   void Set(string name, string label)
   {  
      if (GetIndex(name) > -1) {Change(name, label); return;}
      names[count] = name;
      labels[count] = label;
      MakeLabel(name, x+5, y+5 + count*sdy, label, font_size, text_color, fontname);
      count++;
      ResizeBox(label);
   }
   void Set(string name, string label, int value)
   {
      if (GetIndex(name) > -1) {Change(name, value); return;}
      names[count] = name;
      labels[count] = label;
      digits[count] = 0;
      MakeLabel(name, x+5, y+5 + count*sdy, label + sdts(value, 0), font_size, text_color, fontname);
      count++;
      ResizeBox(label);
   }
   void Set(string name, string label, double value, int _digits=2)
   {
      if (GetIndex(name) > -1) {Change(name, value); return;}
      names[count] = name;
      labels[count] = label;
      digits[count] = _digits;
      MakeLabel(name, x+5, y+5 + count*sdy, label + sdts(value, _digits), font_size, text_color, fontname);
      count++;
      ResizeBox(label);
   }
   void Change(string name, string label)
   {
      ChangeLabel(name, label);
      ResizeBox(label);
   }
   void Change(string name, int value)
   {
      int index = GetIndex(name);
      if (index<0)
         return;
      string s = labels[index] + sdts(value, 0);
      ChangeLabel(name, s);
      ResizeBox(s);
   }
   void Change(string name, double value)
   {
      int index = GetIndex(name);
      if (index<0)
         return;
      string s = labels[index] + sdts(value, digits[index]);
      ChangeLabel(name, s);
      ResizeBox(s);
   }
   string sdts(double value, int d)
   {
      int len = StringLen(DoubleToStr(value, 0));
      spaces = fmax(spaces, len+1);
      string prefix;
      StringInit(prefix, spaces-len, ' ');
      return(prefix + DoubleToStr(value, d));
   }
   int GetIndex(string name)
   {
      for (int i=0; i<count; i++)
         if (names[i] == name)
            return(i);
      return(-1);
   }
   void ResizeBox(string text)
   {
      width = fmax(width, StringLen(text));
      dx = width*sdx + 10;
      dy = count*sdy + 10;
      ObjectSetInteger(0, "textbox", OBJPROP_XSIZE, dx);
      ObjectSetInteger(0, "textbox", OBJPROP_YSIZE, dy);
      WindowRedraw();
   }
   int GetWidth()
   {
      Print("DX=", dx);
      return(dx);
   }
   int GetHeight(int lines)
   {
      Print("DY=", dy);
      return(dy*lines);
   }
   void DrawBox()
   {
      string line;
      string res;
      int i;
      for (i=0; i<dx; i++)
         line += "?";
      line += "/r/n";
      for (i=0; i<dy; i++)
         res += line;
      MakeLabel("tbox", x, y, res, font_size, clrBlue, fontname);
   }
};
Look   info(10, clrWhite, 10, 16);
string SAdd(string accu, string value, string separator=";")
{
   if (StringLen(accu)>0)
      accu += separator;
   if (StringLen(value)<1)
      value = " ";
   //if (value == 0.0)
   //   return(accu+" ");
   return(accu + value);
}
void SaveStr(string s, string filename = "accu")
{
   int hf = FileOpen(filename, FILE_WRITE|FILE_BIN);
   FileWriteString(hf, s);
   FileClose(hf);
}
string LoadStr(string filename = "accu")
{
   int hf = FileOpen(filename, FILE_READ|FILE_BIN);
   string res = FileReadString(hf);
   FileClose(hf);
   return(res);
}
void Button(string caption, string name, int x, int y, int width, int height,  color col)
{
   ObjectCreate(0, name, OBJ_BUTTON, 0, 0, 0);
   ObjectSetInteger(0,name,OBJPROP_XDISTANCE,x);
   ObjectSetInteger(0,name,OBJPROP_YDISTANCE,y);
   ObjectSetInteger(0,name,OBJPROP_XSIZE,width);
   ObjectSetInteger(0,name,OBJPROP_YSIZE,height);
   ObjectSetInteger(0,name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
   ObjectSetString(0, name, OBJPROP_TEXT, caption);
   ObjectSetString(0, name, OBJPROP_FONT, "Lucida Console");
   ObjectSetInteger(0, name, OBJPROP_FONTSIZE, 10);
   ObjectSetInteger(0,name,OBJPROP_COLOR, clrWhite); //цвет текста
   ObjectSetInteger(0,name,OBJPROP_BGCOLOR, col);
   ObjectSetInteger(0,name,OBJPROP_BORDER_COLOR, clrWhite);
   ObjectSetInteger(0,name,OBJPROP_BACK,false);
   ObjectSetInteger(0,name,OBJPROP_SELECTABLE,false);
   ObjectSetInteger(0,name,OBJPROP_SELECTED,false);
   ObjectSetInteger(0,name,OBJPROP_HIDDEN,false);
   ObjectSetInteger(0,name,OBJPROP_ZORDER,1);
}
void Edit(string text, string name, int x, int y, int width, int height,  color col, bool read_only=false)
{
   ObjectCreate(0, name, OBJ_EDIT, 0, 0, 0);
   ObjectSetInteger(0,name,OBJPROP_XDISTANCE,x);
   ObjectSetInteger(0,name,OBJPROP_YDISTANCE,y);
   ObjectSetInteger(0,name,OBJPROP_XSIZE, width);
   ObjectSetInteger(0,name,OBJPROP_YSIZE, height);
   ObjectSetInteger(0,name, OBJPROP_ALIGN, ALIGN_CENTER);
   ObjectSetInteger(0,name, OBJPROP_CORNER, CORNER_LEFT_UPPER);
   ObjectSetString(0, name, OBJPROP_TEXT, text);
   ObjectSetInteger(0,name,OBJPROP_READONLY, read_only);
   ObjectSetString(0, name, OBJPROP_FONT, "Lucida Console");
   ObjectSetInteger(0, name, OBJPROP_FONTSIZE, 10);
   ObjectSetInteger(0,name,OBJPROP_COLOR, clrBlack);
   ObjectSetInteger(0,name,OBJPROP_BGCOLOR, col);
   ObjectSetInteger(0,name,OBJPROP_BORDER_COLOR, clrWhite);
   ObjectSetInteger(0,name,OBJPROP_BACK,false);
   ObjectSetInteger(0,name,OBJPROP_SELECTABLE,false);
   ObjectSetInteger(0,name,OBJPROP_SELECTED,false);
   ObjectSetInteger(0,name,OBJPROP_HIDDEN,false);
   ObjectSetInteger(0,name,OBJPROP_ZORDER,1);
}
void SetEdit(string name, string text)
{
   ObjectSetString(0, name, OBJPROP_TEXT, text);
}
string GetEdit(string name)
{
   return(ObjectGetString(0, name, OBJPROP_TEXT));
}
double OnTester()
{
   if (SaveMap && (is_optimization || is_testing))  //Заполнение карты оптимизации x:TP, y:Step
   {
      string map[100][100];
      string tmp;
      int curTP;
      int curStep = 0;
      string filename = "map " + symbol + ".csv";
      int handle = FileOpen(filename, FILE_READ|FILE_CSV);  //Сначала читаем карту
      //Print("FileOpen Error: ", Error(GetLastError()));
      ResetLastError();
      if(handle!=INVALID_HANDLE)
      {
         while (!FileIsEnding(handle))
         {
            map[curStep][curTP] = FileReadString(handle);
            curTP++;
            if (FileIsLineEnding(handle))
            {
               curTP = 0;
               curStep++;
            }
         }
         FileClose(handle);
      }
      //else
         //Print("FileNotOpen");
      tmp = "=" + DoubleToStr(fmax(0.0, TesterStatistics(STAT_PROFIT)), 2) + "/" + DoubleToStr(fmax(1, TesterStatistics(STAT_EQUITY_DD)), 2);
      StringReplace(tmp, ".", ",");
      map[Step-1][TakeProfit-1] = tmp;
      //Print("------------------| ", tmp);
      //Print("Error: ", Error(GetLastError()));
      handle = FileOpen(filename, FILE_WRITE|FILE_CSV);   //Теперь пишем обновленную карту в файл
      string accu;
      for (int s=0; s<100; s++)
      {
         accu = "";
         for (int t=0; t<100; t++)
         {
            accu = SAdd(accu, map[s][t]);
         }
         FileWrite(handle, accu);
      }
      FileClose(handle);
      //Print("Error: ", Error(GetLastError()));
   }
   return(TesterStatistics(STAT_PROFIT) / fmax(1, TesterStatistics(STAT_EQUITY_DD)));
   //return(fmax(0.0, AccountEquity()-StartEqu) / fmax(1, max_dd));
}
//====================================== Обертки функций работы с ордерами =====================================
//----- Новый ордер -----
void NewOrder()
{
   if (!is_optimization && (!CheckConnect() || !CheckMargin(o_lots)))
      return;
   if (o_lots >= ConfirmLot)
   {
      Print("Открытие большим лотом: ", o_lots);
      int ret=MessageBox("Советник пытается открыть ордер объемом: " + DoubleToStr(o_lots, 2),
      "Открыть ордер?", MB_YESNO|MB_ICONQUESTION|MB_TOPMOST); 
      if (ret==IDNO)
         return;
   }
   if (ECN_Safe)// && showinfo)
   {
      o_slprice = 0;
      o_tpprice = 0;
   }
   if (OrderSend(symbol, o_type, o_lots, o_openprice, Slippage, o_slprice, o_tpprice, StringConcatenate(intret, "-", CommentText), Magic, 0, colors[o_type]) < 0)
   {
      int err=GetLastError();
      ShowError("NewOrder critical: ", err, true);
      Err(err);
   }
}
//----- Модификация ордера -----
void ModOrder()
{
   if (!is_optimization && !CheckConnect())
      return;
   if (!OrderModify(o_ticket, o_openprice, o_slprice, o_tpprice, 0, clrNONE))
   {
      int err=GetLastError();
      if (err==1) return;  //Убираем часто встречающуюся ошибку - "Нет ошибки" =)
      ShowError("ModOrder critical: ", err, true);
      Err(err);
   }
}
//----- Закрыть ордер -----
void CloseOrder()
{
   if (!is_optimization && !CheckConnect())
      return;
   if (!OrderClose(o_ticket, o_lots, o_openprice, Slippage, clrGreen))
   {
      int err=GetLastError();
      ShowError("CloseOrder critical: ", err, true);
      Err(err);
   }
}
//----- Удалить ордер -----
void DelOrder()
{
   if (!is_optimization && !CheckConnect())
      return;
   if (!OrderDelete(o_ticket))
   {
      int err=GetLastError();
      ShowError("DelOrder critical: ", err, true);
      Err(err);
   }
}
//Проверяем свободную маржу
bool CheckMargin(double lot)
{
   if (lot * req_margin > AccountFreeMargin())
   {
       ShowError("CheckMargin: Недостаточно средств для открытия позиции");
       return(false);
   }
   return(true);
}
//Проверяем проблемы со связью или с каналом
bool CheckConnect()
{
   if(!IsConnected())
   {
       ShowError("CheckConnect: Cвязь с сервером отсутствует или прервана");
       timeout = fmax(timeout, 5000);
       return(false);
   }
   if(IsTradeContextBusy())
   {
       ShowError("CheckConnect: Поток для выполнения торговых операций занят");
       timeout = fmax(timeout, 10000);
       return(false);
   }
   if(!IsTradeAllowed())
   {
       ShowError("CheckConnect: Эксперту запрещено торговать или поток занят");
       timeout = fmax(timeout, 10000);
       return(false);
   }
   return(true);
}
//Обработать ошибку
void ShowError(string message, int err=-1, bool critical=false)
{
   if (is_optimization)
      return;
   if (critical || ShowDebug)
   {
      Print(message, Error(err));
      FileWrite(herror, "-<[ ",TimeCurrent(), " ]>-----------------------------------------------------------");
      FileWrite(herror, message);
      FileWrite(herror, "Ask/Bid: ", Ask, "/", Bid);
      FileWrite(herror, "Next type: ", o_type, ", ticket: ", o_ticket, ", lots: ", o_lots,
         ", open price: ", o_openprice, ", tp price: ", o_tpprice, ", sl price", o_slprice);
   }
   timeout = fmax(timeout, 1000);  //При любой ошибке минимум секунду задержки
}
//+------------------------------------------------------------------+
//|Обработка ошибок по коду в соответствии с рекомендациями          |
//+------------------------------------------------------------------+
void Err(int id)
{
   if(id==4 || id==132) timeout = fmax(timeout, 60000);
   if(id==6 || id==129 || id==130 || id==136) timeout = fmax(timeout, 5000);
   if(id==128 || id==142 || id==143) timeout = fmax(timeout, 60000);
   if(id==145) timeout = fmax(timeout, 15000);
   if(id==146) timeout = fmax(timeout, 10000);
}

//+------------------------------------------------------------------+
//|Возвращает описание ошибки на русском                             |
//+------------------------------------------------------------------+
string Error(int id)
  {
   string res="";
   switch(id)
     {
      case -1:   res=""; break;
      case 0:    res=" Нет ошибок. "; break;
      case 1:    res=" Нет ошибки, но результат неизвестен. "; break;
      case 2:    res=" Общая ошибка. "; break;
      case 3:    res=" Неправильные параметры. "; break;
      case 4:    res=" Торговый сервер занят. "; break;
      case 5:    res=" Старая версия клиентского терминала. "; break;
      case 6:    res=" Нет связи с торговым сервером. "; break;
      case 7:    res=" Недостаточно прав. "; break;
      case 8:    res=" Слишком частые запросы. "; break;
      case 9:    res=" Недопустимая операция нарушающая функционирование сервера. "; break;
      case 64:   res=" Счет заблокирован. "; break;
      case 65:   res=" Неправильный номер счета. "; break;
      case 128:  res=" Истек срок ожидания совершения сделки. "; break;
      case 129:  res=" Неправильная цена. "; break;
      case 130:  res=" Неправильные стопы. "; break;
      case 131:  res=" Неправильный объем. "; break;
      case 132:  res=" Рынок закрыт. "; break;
      case 133:  res=" Торговля запрещена. "; break;
      case 134:  res=" Недостаточно денег для совершения операции. "; break;
      case 135:  res=" Цена изменилась. "; break;
      case 136:  res=" Нет цен. "; break;
      case 137:  res=" Брокер занят. "; break;
      case 138:  res=" Новые цены. "; break;
      case 139:  res=" Ордер заблокирован и уже обрабатывается. "; break;
      case 140:  res=" Разрешена только покупка. "; break;
      case 141:  res=" Слишком много запросов. "; break;
      case 145:  res=" Модификация запрещена, так как ордер слишком близок к рынку. "; break;
      case 146:  res=" Подсистема торговли занята. "; break;
      case 147:  res=" Использование даты истечения ордера запрещено брокером. "; break;
      case 148:  res=" Количество открытых и отложенных ордеров достигло предела, установленного брокером. "; break;
      case 4000: res=" Нет ошибки. "; break;
      case 4001: res=" Неправильный указатель функции. "; break;
      case 4002: res=" Индекс массива - вне диапазона. "; break;
      case 4003: res=" Нет памяти для стека функций. ";break;
      case 4004: res=" Переполнение стека после рекурсивного вызова. "; break;
      case 4005: res=" На стеке нет памяти для передачи параметров. "; break;
      case 4006: res=" Нет памяти для строкового параметра. "; break;
      case 4007: res=" Нет памяти для временной строки. "; break;
      case 4008: res=" Неинициализированная строка. "; break;
      case 4009: res=" Неинициализированная строка в массиве. ";break;
      case 4010: res=" Нет памяти для строкового массива. "; break;
      case 4011: res=" Слишком длинная строка. "; break;
      case 4012: res=" Остаток от деления на ноль. "; break;
      case 4013: res=" Деление на ноль. "; break;
      case 4014: res=" Неизвестная команда. "; break;
      case 4015: res=" Неправильный переход. "; break;
      case 4016: res=" Неинициализированный массив. "; break;
      case 4017: res=" Вызовы DLL не разрешены. "; break;
      case 4018: res=" Невозможно загрузить библиотеку. "; break;
      case 4019: res=" Невозможно вызвать функцию. "; break;
      case 4020: res=" Вызовы внешних библиотечных функций не разрешены. "; break;
      case 4021: res=" Недостаточно памяти для строки, возвращаемой из функции. "; break;
      case 4022: res=" Система занята. "; break;
      case 4050: res=" Неправильное количество параметров функции. "; break;
      case 4051: res=" Недопустимое значение параметра функции. "; break;
      case 4052: res=" Внутренняя ошибка строковой функции. "; break;
      case 4053: res=" Ошибка массива. "; break;
      case 4054: res=" Неправильное использование массива-таймсерии. "; break;
      case 4055: res=" Ошибка пользовательского индикатора. "; break;
      case 4056: res=" Массивы несовместимы. "; break;
      case 4057: res=" Ошибка обработки глобальныех переменных. "; break;
      case 4058: res=" Глобальная переменная не обнаружена. "; break;
      case 4059: res=" Функция не разрешена в тестовом режиме. "; break;
      case 4060: res=" Функция не разрешена. "; break;
      case 4061: res=" Ошибка отправки почты. "; break;
      case 4062: res=" Ожидается параметр типа string. "; break;
      case 4063: res=" Ожидается параметр типа integer. "; break;
      case 4064: res=" Ожидается параметр типа double. "; break;
      case 4065: res=" В качестве параметра ожидается массив. "; break;
      case 4066: res=" Запрошенные исторические данные в состоянии обновления. "; break;
      case 4067: res=" Ошибка при выполнении торговой операции. "; break;
      case 4099: res=" Конец файла. "; break;
      case 4100: res=" Ошибка при работе с файлом. "; break;
      case 4101: res=" Неправильное имя файла. "; break;
      case 4102: res=" Слишком много открытых файлов. "; break;
      case 4103: res=" Невозможно открыть файл. "; break;
      case 4104: res=" Несовместимый режим доступа к файлу. "; break;
      case 4105: res=" Ни один ордер не выбран. "; break;
      case 4106: res=" Неизвестный символ. "; break;
      case 4107: res=" Неправильный параметр цены для торговой функции. "; break;
      case 4108: res=" Неверный номер тикета. "; break;
      case 4109: res=" Торговля не разрешена. Необходимо включить опцию Разрешить советнику торговать в свойствах эксперта. "; break;
      case 4110: res=" Длинные позиции не разрешены. Необходимо проверить свойства эксперта. "; break;
      case 4111: res=" Короткие позиции не разрешены. Необходимо проверить свойства эксперта. "; break;
      case 4200: res=" Объект уже существует. "; break;
      case 4201: res=" Запрошено неизвестное свойство объекта. "; break;
      case 4202: res=" Объект не существует. "; break;
      case 4203: res=" Неизвестный тип объекта. "; break;
      case 4204: res=" Нет имени объекта. "; break;
      case 4205: res=" Ошибка координат объекта. "; break;
      case 4206: res=" Не найдено указанное подокно. "; break;
      case 200 : res=" Ордер не найден. "; break;
      case 201 : res=" Стоплосс или тейкпрофит слишком близок к цене открытия. "; break;
      case 202 : res=" Цена открытия отложенного ордера не верна. "; break;
      default :  res=" Неизвестная ошибка. ";
     }
   return(res);
  }
