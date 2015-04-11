void CriticalError(string msg)
{
	if (!showinfo) return;
	info.Set("5", "Advisor stopped");
	info.Set("6", msg);
}

//���������� ������
void ShowError(string message, int err = -1, bool critical = false) {
	if (mqlOptimization)
		return;

	if (critical || ShowDebug)
	{
		Print(message, Error(err));
		FileWrite(herror, "-<[ ", TimeCurrent(), " ]>-----------------------------------------------------------");
		FileWrite(herror, message);
		FileWrite(herror, "Ask/Bid: ", Ask, "/", Bid);
	}

	timeout = fmax(timeout, 1000);  //��� ����� ������ ������� ������� ��������
}

void ShowActionError(TradeAction& action, string message, int err = -1, bool critical = false)
{
	if (mqlOptimization)
		return;

	if (critical || ShowDebug)
	{
		Print(message, Error(err));
		FileWrite(herror, "-<[ ", TimeCurrent(), " ]>-----------------------------------------------------------");
		FileWrite(herror, StringConcatenate("Error ", err, ": ", message));
		FileWrite(herror, "Ask/Bid: ", Ask, "/", Bid);
		FileWrite(herror, 
			"Next type: ",    action.o_type, 
			", ticket: ",     action.o_ticket, 
			", lots: ",       action.o_lots, 
			", open price: ", action.o_openprice,
			", tp price: ",   action.o_tpprice, 
			", sl price: ",   action.o_slprice
		);
	}

	timeout = fmax(timeout, 1000);  //��� ����� ������ ������� ������� ��������
}
void ShowActionError2(TradeAction& oldaction, TradeAction& action, string message, int err = -1, bool critical = false)
{
	if (mqlOptimization)
		return;

	if (critical || ShowDebug)
	{
		Print(message, Error(err));
		FileWrite(herror, "-<[ ", TimeCurrent(), " ]>-----------------------------------------------------------");
		FileWrite(herror, StringConcatenate("Error ", err, ": ", message));
		FileWrite(herror, "Ask/Bid: ", Ask, "/", Bid);
		FileWrite(herror, 
			"Next type: ",    oldaction.o_type, 
			", ticket: ",     oldaction.o_ticket, 
			", lots: ",       oldaction.o_lots, 
			", open price: ", oldaction.o_openprice,
			", tp price: ",   oldaction.o_tpprice, 
			", sl price: ",   oldaction.o_slprice,
			" =>"
		);
		FileWrite(herror, 
			"Next type: ",    action.o_type, 
			", ticket: ",     action.o_ticket, 
			", lots: ",       action.o_lots, 
			", open price: ", action.o_openprice,
			", tp price: ",   action.o_tpprice, 
			", sl price: ",   action.o_slprice
		);
	}

	timeout = fmax(timeout, 1000);  //��� ����� ������ ������� ������� ��������
}

// ��������� ������ �� ���� � ������������ � ��������������
void Err(int id)
{
	switch (id) {
		case 4 :
		case 132 :
			timeout = fmax(timeout, 60000);
			break;
		case 6 :
		case 129 :
		case 130 :
		case 136 :
			timeout = fmax(timeout, 5000);
			break;
		case 128 :
		case 142 :
		case 143 :
			timeout = fmax(timeout, 60000);
			break;
		case 145 :
			timeout = fmax(timeout, 15000);
			break;
		case 146 :
			timeout = fmax(timeout, 10000);
			break;
	}
}

// ���������� �������� ������ �� �������
string Error(int id)
{
	string res = "";

	switch (id) {
		case -1:   res=""; break;
		case 0:    res=" ��� ������. "; break;
		case 1:    res=" ��� ������, �� ��������� ����������. "; break;
		case 2:    res=" ����� ������. "; break;
		case 3:    res=" ������������ ���������. "; break;
		case 4:    res=" �������� ������ �����. "; break;
		case 5:    res=" ������ ������ ����������� ���������. "; break;
		case 6:    res=" ��� ����� � �������� ��������. "; break;
		case 7:    res=" ������������ ����. "; break;
		case 8:    res=" ������� ������ �������. "; break;
		case 9:    res=" ������������ �������� ���������� ���������������� �������. "; break;
		case 64:   res=" ���� ������������. "; break;
		case 65:   res=" ������������ ����� �����. "; break;
		case 128:  res=" ����� ���� �������� ���������� ������. "; break;
		case 129:  res=" ������������ ����. "; break;
		case 130:  res=" ������������ �����. "; break;
		case 131:  res=" ������������ �����. "; break;
		case 132:  res=" ����� ������. "; break;
		case 133:  res=" �������� ���������. "; break;
		case 134:  res=" ������������ ����� ��� ���������� ��������. "; break;
		case 135:  res=" ���� ����������. "; break;
		case 136:  res=" ��� ���. "; break;
		case 137:  res=" ������ �����. "; break;
		case 138:  res=" ����� ����. "; break;
		case 139:  res=" ����� ������������ � ��� ��������������. "; break;
		case 140:  res=" ��������� ������ �������. "; break;
		case 141:  res=" ������� ����� ��������. "; break;
		case 145:  res=" ����������� ���������, ��� ��� ����� ������� ������ � �����. "; break;
		case 146:  res=" ���������� �������� ������. "; break;
		case 147:  res=" ������������� ���� ��������� ������ ��������� ��������. "; break;
		case 148:  res=" ���������� �������� � ���������� ������� �������� �������, �������������� ��������. "; break;
		case 4000: res=" ��� ������. "; break;
		case 4001: res=" ������������ ��������� �������. "; break;
		case 4002: res=" ������ ������� - ��� ���������. "; break;
		case 4003: res=" ��� ������ ��� ����� �������. ";break;
		case 4004: res=" ������������ ����� ����� ������������ ������. "; break;
		case 4005: res=" �� ����� ��� ������ ��� �������� ����������. "; break;
		case 4006: res=" ��� ������ ��� ���������� ���������. "; break;
		case 4007: res=" ��� ������ ��� ��������� ������. "; break;
		case 4008: res=" �������������������� ������. "; break;
		case 4009: res=" �������������������� ������ � �������. ";break;
		case 4010: res=" ��� ������ ��� ���������� �������. "; break;
		case 4011: res=" ������� ������� ������. "; break;
		case 4012: res=" ������� �� ������� �� ����. "; break;
		case 4013: res=" ������� �� ����. "; break;
		case 4014: res=" ����������� �������. "; break;
		case 4015: res=" ������������ �������. "; break;
		case 4016: res=" �������������������� ������. "; break;
		case 4017: res=" ������ DLL �� ���������. "; break;
		case 4018: res=" ���������� ��������� ����������. "; break;
		case 4019: res=" ���������� ������� �������. "; break;
		case 4020: res=" ������ ������� ������������ ������� �� ���������. "; break;
		case 4021: res=" ������������ ������ ��� ������, ������������ �� �������. "; break;
		case 4022: res=" ������� ������. "; break;
		case 4050: res=" ������������ ���������� ���������� �������. "; break;
		case 4051: res=" ������������ �������� ��������� �������. "; break;
		case 4052: res=" ���������� ������ ��������� �������. "; break;
		case 4053: res=" ������ �������. "; break;
		case 4054: res=" ������������ ������������� �������-���������. "; break;
		case 4055: res=" ������ ����������������� ����������. "; break;
		case 4056: res=" ������� ������������. "; break;
		case 4057: res=" ������ ��������� ����������� ����������. "; break;
		case 4058: res=" ���������� ���������� �� ����������. "; break;
		case 4059: res=" ������� �� ��������� � �������� ������. "; break;
		case 4060: res=" ������� �� ���������. "; break;
		case 4061: res=" ������ �������� �����. "; break;
		case 4062: res=" ��������� �������� ���� string. "; break;
		case 4063: res=" ��������� �������� ���� integer. "; break;
		case 4064: res=" ��������� �������� ���� double. "; break;
		case 4065: res=" � �������� ��������� ��������� ������. "; break;
		case 4066: res=" ����������� ������������ ������ � ��������� ����������. "; break;
		case 4067: res=" ������ ��� ���������� �������� ��������. "; break;
		case 4099: res=" ����� �����. "; break;
		case 4100: res=" ������ ��� ������ � ������. "; break;
		case 4101: res=" ������������ ��� �����. "; break;
		case 4102: res=" ������� ����� �������� ������. "; break;
		case 4103: res=" ���������� ������� ����. "; break;
		case 4104: res=" ������������� ����� ������� � �����. "; break;
		case 4105: res=" �� ���� ����� �� ������. "; break;
		case 4106: res=" ����������� ������. "; break;
		case 4107: res=" ������������ �������� ���� ��� �������� �������. "; break;
		case 4108: res=" �������� ����� ������. "; break;
		case 4109: res=" �������� �� ���������. ���������� �������� ����� ��������� ��������� ��������� � ��������� ��������. "; break;
		case 4110: res=" ������� ������� �� ���������. ���������� ��������� �������� ��������. "; break;
		case 4111: res=" �������� ������� �� ���������. ���������� ��������� �������� ��������. "; break;
		case 4200: res=" ������ ��� ����������. "; break;
		case 4201: res=" ��������� ����������� �������� �������. "; break;
		case 4202: res=" ������ �� ����������. "; break;
		case 4203: res=" ����������� ��� �������. "; break;
		case 4204: res=" ��� ����� �������. "; break;
		case 4205: res=" ������ ��������� �������. "; break;
		case 4206: res=" �� ������� ��������� �������. "; break;
		case 200 : res=" ����� �� ������. "; break;
		case 201 : res=" �������� ��� ���������� ������� ������ � ���� ��������. "; break;
		case 202 : res=" ���� �������� ����������� ������ �� �����. "; break;
		default :  res=" ����������� ������. ";
	}

	return (res);
}