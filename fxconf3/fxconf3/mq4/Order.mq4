class Order
{

public:

	static void Create(TradeAction& action)
	{
		if (!mqlOptimization && (!CheckConnect() || !CheckMargin(action.o_lots)))
			return;

		if (ecn_mode) {
			action.o_slprice = 0;
			action.o_tpprice = 0;
		}

		int ticket = OrderSend(
			symbolName,
			action.o_type, action.o_lots, action.o_openprice,
			Slippage,
			action.o_slprice, action.o_tpprice,
			StringConcatenate(action.comment),
			action.magic, 0, colors[action.o_type]
		);

		if (ticket < 0) {
			int err = GetLastError();
			ShowActionError(action, "NewOrder critical: ", err, true);
			Err(err);
		}
		else if (!mqlOptimization && !mqlTester) {
			OrderSelect(ticket, SELECT_BY_TICKET);
			c_onOrderOpen(ticket, OrderOpenTime());
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
			if (err == 1) //������� ����� ������������� ������ - "��� ������" =)
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
		if(mqlOptimization)
			return;
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
		if(mqlOptimization)
			return;
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
		//Print("Comment: \"", ExtractString(action.comment), "\"");
		//Print("buflen: ", StringBufferLen(action.comment), ", len: ", StringLen(action.comment));

		info.Set((string) action.o_ticket, ExtractString(action.comment));
	}

	static void MsgBox(TradeAction& action)
	{}
	


private:

	// ��������� ��������� �����
	static bool CheckMargin(double lot)
	{
		if (lot * symbolMarginRequired > AccountFreeMargin()) {
			ShowError("CheckMargin: ������������ ������� ��� �������� �������");
			return (false);
		}

		return (true);
	}

	// ��������� �������� �� ������ ��� � �������
	static bool CheckConnect()
	{
		if (!IsConnected()) {
			ShowError("CheckConnect: C���� � �������� ����������� ��� ��������");
			timeout = fmax(timeout, 5000);
			return (false);
		}

		if (IsTradeContextBusy()) {
			ShowError("CheckConnect: ����� ��� ���������� �������� �������� �����");
			timeout = fmax(timeout, 10000);
			return (false);
		}
		
		if (!IsTradeAllowed()) {
			ShowError("CheckConnect: �������� ��������� ��������� ��� ����� �����");
			timeout = fmax(timeout, 10000);
			return (false);
		}

		return (true);
	}

};