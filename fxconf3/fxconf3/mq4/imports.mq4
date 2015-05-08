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
#import "{{FILE}}.dll"
	bool   c_init();
	void   c_deinit();
	void   c_postInit();

	void   c_setint(string prop, int value);
	void   c_setdouble(string prop, double value);
	void   c_setstring(string prop, string value);
	void   c_setvar(string prop, bool& var);
	void   c_setvar(string prop, int& var);
	void   c_setvar(string prop, int& var[]);
	void   c_setvar(string prop, long& var);
	void   c_setvar(string prop, double& var);
	void   c_setvar(string prop, double& var[]);

	void   c_setactions(TradeAction& var[], int length);
	int    c_updateAccount(double balance, double equity, double profit);
	int    c_updateOrder(int ticket, datetime opentime, datetime closetime, double profit);
	void   c_onOrderOpen(int ticket, datetime opentime);

	int    c_add_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0);
	double c_norm_lot(double _lots);
	int    c_getjob();
	int    c_getdpi();
	int    c_get_next_closed();

	void   c_refresh_chartdata(int timeframe, int length, MqlRates& rates[]);
	int    c_get_timeframes(int& timeframes[], int& sizes[]);
	bool   c_tick_init_begin(double ask, double bid, double equity, double balance);
	void   c_tick_init_end();
#import
string ExtractString(string str) {
	int len = StringLen(str);
	for (int i = 0; i < len; i++) {
		if (StringGetChar(str, i) == 0) {
			return StringSubstr(str, 0, i);
		}
	}
	return str;
}
