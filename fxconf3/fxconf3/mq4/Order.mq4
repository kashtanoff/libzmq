class Order
{

public:

	static void Create(TradeAction& action)
	{
		if (!mqlOptimization && (!CheckConnect() || !CheckMargin(action.o_lots)))
			return;

		if (
			OrderSend(
				symbolName, 
				action.o_type, action.o_lots, action.o_openprice, 
				Slippage, 
				action.o_slprice, action.o_tpprice,
				StringConcatenate(action.intret, "-", CommentText),
				Magic, 0, colors[action.o_type]
			) < 0
		) {
			int err = GetLastError();
			ShowActionError(action, "NewOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Modify(TradeAction& action)
	{
		if (!mqlOptimization && !CheckConnect())
			return;

		TradeAction a;
		if (true == OrderSelect(action.o_ticket, SELECT_BY_TICKET)) {
			a.o_ticket    = OrderTicket();
			a.o_type      = OrderType();
			a.o_lots      = OrderLots();
			a.o_openprice = OrderOpenPrice();
			a.o_slprice   = OrderStopLoss();
			a.o_tpprice   = OrderTakeProfit();
		}

		if (!OrderModify(action.o_ticket, action.o_openprice, action.o_slprice, action.o_tpprice, 0, clrNONE)) {
			int err = GetLastError();
			if (err == 1) //Убираем часто встречающуюся ошибку - "Нет ошибки" =)
				return;

			ShowActionError2(a, action, "ModOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Delete(TradeAction& action)
	{
		if (!mqlOptimization && !CheckConnect())
			return;

		if (!OrderDelete(action.o_ticket)) {
			int err = GetLastError();
			ShowActionError(action, "DelOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Close(TradeAction& action)
	{
		if (!mqlOptimization && !CheckConnect())
			return;

		if (!OrderClose(action.o_ticket, action.o_lots, action.o_openprice, Slippage, clrGreen)) {
			int err = GetLastError();
			ShowActionError(action, "CloseOrder critical: ", err, true);
			Err(err);
		}
	}
	static void PrintOrder(TradeAction& action)
	{
	   Print("ticket: ", action.o_ticket,
	         ", type: ", action.o_type, 
	         ", price: ", action.o_openprice,
	         ", tp: ", action.o_tpprice,
	         ", sl: ", action.o_slprice,
	         ", lots: ", action.o_lots,
	         " - ", action.comment);
	}
	static void PrintText(TradeAction& action)
	{
	   Print(action.comment);
	}
	static void DrawOrder(TradeAction& action)
	{
	   if(!showinfo)
	      return;
	   if (!OrderSelect(action.o_ticket, SELECT_BY_TICKET, MODE_HISTORY))
		   return;
      info.DrawOrder(action.o_ticket, OrderType(), OrderOpenTime(), OrderOpenPrice(), OrderCloseTime(), OrderClosePrice());
      info.DelOldDraws();
	}
	static void ShowValue(TradeAction& action)  //o_type: 0-str, 1-int, 2-double; o_ticket - key; comment - label; intret - int value/digits; o_lots - double value
	{
	   if(!showinfo)
	      return;
	   switch(action.o_type) {
	      case 0: info.Set((string)action.o_ticket, action.comment); break;
	      case 1: info.Set((string)action.o_ticket, action.intret); break;
	      case 2: info.Set((string)action.o_ticket, action.o_lots, action.intret); break;
	   }
	}
	static void MsgBox(TradeAction& action)
	{}
	


private:

	// Проверяем свободную маржу
	static bool CheckMargin(double lot)
	{
		if (lot * symbolMarginRequired > AccountFreeMargin()) {
			ShowError("CheckMargin: Недостаточно средств для открытия позиции");
			return (false);
		}

		return (true);
	}

	// Проверяем проблемы со связью или с каналом
	static bool CheckConnect()
	{
		if (!IsConnected()) {
			ShowError("CheckConnect: Cвязь с сервером отсутствует или прервана");
			timeout = fmax(timeout, 5000);
			return (false);
		}

		if (IsTradeContextBusy()) {
			ShowError("CheckConnect: Поток для выполнения торговых операций занят");
			timeout = fmax(timeout, 10000);
			return (false);
		}
		
		if (!IsTradeAllowed()) {
			ShowError("CheckConnect: Эксперту запрещено торговать или поток занят");
			timeout = fmax(timeout, 10000);
			return (false);
		}

		return (true);
	}

};