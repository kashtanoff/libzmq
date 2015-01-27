class Order
{
public:
	static void Create()
	{
		if (!is_optimization && (!CheckConnect() || !CheckMargin(o_lots)))
			return;

		if (o_lots >= ConfirmLot)
		{
			Print("Открытие большим лотом: ", o_lots);
			int ret = MessageBox(
				"Советник пытается открыть ордер объемом: " + DoubleToStr(o_lots, 2),
				"Открыть ордер?", 
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST
			);
			if (ret == IDNO)
				return;
		}
	
		if (ECN_Safe)// && showinfo)
		{
			o_slprice = 0;
			o_tpprice = 0;
		}
	
		if (
			OrderSend(
				symbol, 
				o_type, 
				o_lots, 
				o_openprice, 
				Slippage, 
				o_slprice, 
				o_tpprice, 
				StringConcatenate(intret, "-", CommentText), 
				Magic, 
				0, 
				colors[o_type]
			) < 0
		) {
			int err = GetLastError();
			ShowError("NewOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Modify()
	{
		if (!is_optimization && !CheckConnect())
			return;

		if (!OrderModify(o_ticket, o_openprice, o_slprice, o_tpprice, 0, clrNONE))
		{
			int err = GetLastError();
			if (err == 1) //Убираем часто встречающуюся ошибку - "Нет ошибки" =)
				return;

			ShowError("ModOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Close()
	{
		if (!is_optimization && !CheckConnect())
			return;

		if (!OrderClose(o_ticket, o_lots, o_openprice, Slippage, clrGreen))
		{
			int err = GetLastError();
			ShowError("CloseOrder critical: ", err, true);
			Err(err);
		}
	}

	static void Delete()
	{
		if (!is_optimization && !CheckConnect())
			return;

		if (!OrderDelete(o_ticket))
		{
			int err = GetLastError();
			ShowError("DelOrder critical: ", err, true);
			Err(err);
		}
	}

private:
	// Проверяем свободную маржу
	static bool CheckMargin(double lot)
	{
		if (lot * req_margin > AccountFreeMargin())
		{
			ShowError("CheckMargin: Недостаточно средств для открытия позиции");
			return (false);
		}

		return (true);
	}

	// Проверяем проблемы со связью или с каналом
	static bool CheckConnect()
	{
		if (!IsConnected())
		{
			ShowError("CheckConnect: Cвязь с сервером отсутствует или прервана");
			timeout = fmax(timeout, 5000);
			return (false);
		}

		if (IsTradeContextBusy())
		{
			ShowError("CheckConnect: Поток для выполнения торговых операций занят");
			timeout = fmax(timeout, 10000);
			return (false);
		}
		
		if (!IsTradeAllowed())
		{
			ShowError("CheckConnect: Эксперту запрещено торговать или поток занят");
			timeout = fmax(timeout, 10000);
			return (false);
		}

		return (true);
	}
};