class Order
{

public:

	static void Create(TradeAction& action)
	{
		if (!is_optimization && (!CheckConnect() || !CheckMargin(action.o_lots)))
			return;

		if (action.o_lots >= ConfirmLot) {
			Print("Открытие большим лотом: ", action.o_lots);
			int ret = MessageBox(
				"Советник пытается открыть ордер объемом: " + DoubleToStr(action.o_lots, 2),
				"Открыть ордер?", 
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST
			);
			if (ret == IDNO)
				return;
		}
	
		if (
			OrderSend(
				symbol, 
				action.o_type, action.o_lots, action.o_openprice, 
				Slippage, 0, 0,
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
		if (!is_optimization && !CheckConnect())
			return;

		if (!OrderModify(action.o_ticket, action.o_openprice, action.o_slprice, action.o_tpprice, 0, clrNONE)) {
			int err = GetLastError();
			if (err == 1) //Убираем часто встречающуюся ошибку - "Нет ошибки" =)
				return;

			ShowActionError(action, "ModOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Delete(TradeAction& action)
	{
		if (!is_optimization && !CheckConnect())
			return;

		if (!OrderDelete(action.o_ticket)) {
			int err = GetLastError();
			ShowActionError(action, "DelOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Close(TradeAction& action)
	{
		if (!is_optimization && !CheckConnect())
			return;

		if (!OrderClose(action.o_ticket, action.o_lots, action.o_openprice, Slippage, clrGreen)) {
			int err = GetLastError();
			ShowActionError(action, "CloseOrder critical: ", err, true);
			Err(err);
		}
	}

private:

	// Проверяем свободную маржу
	static bool CheckMargin(double lot)
	{
		if (lot * req_margin > AccountFreeMargin()) {
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