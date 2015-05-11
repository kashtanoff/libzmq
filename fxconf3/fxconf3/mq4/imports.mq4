struct TradeAction {
	double o_lots;      // 8 bytes
	double o_openprice; // 8 bytes
	double o_tpprice;   // 8 bytes
	double o_slprice;   // 8 bytes
	int    o_ticket;    // 4 bytes
	int    o_type;      // 4 bytes
	int    intret;      // 4 bytes
	int    actionId;    // 4 bytes
	string comment;     // 12 bytes
};

struct TfRates {
	int timeframe;
	int length;
	MqlRates rates[];
};

string ExtractString(string str) {
	int len = StringLen(str);
	for (int i = 0; i < len; i++) {
		if (StringGetChar(str, i) == 0) {
			return StringSubstr(str, 0, i);
		}
	}
	return str;
}
enum Timeframes {
	M1  = 1,		//M1
	M5  = 5,		//M5
	M15 = 15,		//M15
	M30 = 30,		//M30
	H1  = 60,		//H1
	H4  = 240,		//H4
	D1  = 1440,		//D1
	W1  = 10080,	//W1
	MN1 = 43200		//MN1
}