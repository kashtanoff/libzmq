class Order
{
public:
	static void Create()
	{
		if (!is_optimization && (!CheckConnect() || !CheckMargin(o_lots)))
			return;

		if (o_lots >= ConfirmLot)
		{
			Print("�������� ������� �����: ", o_lots);
			int ret = MessageBox(
				"�������� �������� ������� ����� �������: " + DoubleToStr(o_lots, 2),
				"������� �����?", 
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
			if (err == 1) //������� ����� ������������� ������ - "��� ������" =)
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
	// ��������� ��������� �����
	static bool CheckMargin(double lot)
	{
		if (lot * req_margin > AccountFreeMargin())
		{
			ShowError("CheckMargin: ������������ ������� ��� �������� �������");
			return (false);
		}

		return (true);
	}

	// ��������� �������� �� ������ ��� � �������
	static bool CheckConnect()
	{
		if (!IsConnected())
		{
			ShowError("CheckConnect: C���� � �������� ����������� ��� ��������");
			timeout = fmax(timeout, 5000);
			return (false);
		}

		if (IsTradeContextBusy())
		{
			ShowError("CheckConnect: ����� ��� ���������� �������� �������� �����");
			timeout = fmax(timeout, 10000);
			return (false);
		}
		
		if (!IsTradeAllowed())
		{
			ShowError("CheckConnect: �������� ��������� ��������� ��� ����� �����");
			timeout = fmax(timeout, 10000);
			return (false);
		}

		return (true);
	}
};