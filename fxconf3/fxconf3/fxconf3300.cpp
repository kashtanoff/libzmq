// fxconf.cpp: ���������� ���������������� ������� ��� ���������� DLL.
//

#include "stdafx.h"
#include "math.h"
#include <stdio.h>
#include <windows.h>
#include <excpt.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <mutex>

#define	debug	1

#pragma region ����� ��������� � �����������
#define	num_orders		100
#define OP_BUY			0
#define OP_SELL			1
#define OP_BUYLIMIT		2
#define OP_SELLLIMIT	3
#define OP_BUYSTOP		4
#define OP_SELLSTOP		5
#define Pool_Size		40
#define max_bars		100

static std::mutex m;

struct ORDER   //��������� ����������� �����, ��� �������� ���������� �������
{
	int		ticket;
	int		type;
	int		magic;
	double	lots;
	double	openprice;
	double	tpprice;
	double	slprice;
	double	closeprice;
	double	profit;
	bool	checked;
};
bool	first_run = true;
typedef std::vector<ORDER*> ORDERLIST;
int		sign[6] = {1, -1, 1, -1, 1, -1};	//����� ��� ������ ����� ��������
double	minmax[2] = {0.0, 1000000.0};		//���� ��������� ����� ��� ���������� ��� �� ������� � �������
double	maxmin[2] = {1000000.0, 0.0};		//����, �� ��������

using std::ostringstream;
using std::ostream;

bool bp;
ostream &msg_box(ostream &s) {
    ostringstream &os = dynamic_cast<ostringstream &>(s);
	if (bp)
		MessageBoxA(NULL, os.str().c_str(), "fxconf3.dll", MB_OK);
	os.swap(ostringstream());
    return s;
}
ostringstream msg;


#pragma endregion
//----------------------------------------------------------------------------
class PARAMETERS 
{
public:
	//��������� �����
	double point;			    //1 �������� ������������ ���� ����
	int    digits;				//2 ���������� ���������� ������ ��� �����������
	char   symbol[10];			//3 �������� �������
	double lot_step;			//4 ����������� ��� ���������� ����
	double lot_min;				//5 ����������� ���
	double lot_max;				//6 ������������ ���
	double min_sl_tp;			//7 ����������� ���������� �� ��������� ��� �����������
	double freeze;				//8 ���������� ��������� �������
	int    is_optimization;		//9 ���� �����������
	int    is_visual;			//10 ���� ����������� ������
	int    is_testing;			//11 ���� ������������
	//��������� ���������
	int		_stop_new[2];		//50, 51 ���������� �������� ����� �����
	int		_stop_avr[2];		//52, 53 ���������� �������� ����� �������
	int		_max_grid_lvl;		//54 ������������ ������� �����
	double	_step;				//55 ������� ���, ����������� ��� (��� �������� �����)
	//double	_step_mult;			//56 ��������� ����
	double	_takeprofit;        //57 ������� ����������, ����������� (��� �������� �����)
	//double	_tp_mult;			//58 ��������� �����������
	int		_forward_lvl;		//59 � ������ ������ ���������� ���������� ������
	//double	_tr_stop;			//60 �������� �������� �����
	//double	_tr_stop_mult;		//61 ��������� �������� �����
	//double  _tr_step;			//62 �������� ��������� ����
	//double  _tr_step_mult;		//63 ��������� ��������� ����
	int		_av_lvl;			//64 ������� ����������
	double	_av_lot;			//65 ��� � �������� ���������� ����������� ������� ����������
	int		_op_av_lvl;			//66 ������� ������ ���������������� ����������
	double	_pips_mult;			//67 ��������� �������
	int		_safe_copy;			//68 ����� ������ ������� � �������� ���� � �� � ����������
	double	_sell_lot;			//69 ��������� ��� �� �������
	double	_buy_lot;			//97 ��������� ��� �� �������
	double  _maxlot;			//70 ������������ ���
	double	_lot_hadge_mult;	//71 ������� ������������
	double	_regres_mult;		//72 ������� ���������
	int		_trend_lvl;			//73
	double	_trend_lot_mult;	//74
	double	_trend_progress;	//75
	int		_repeat_lvl;		//76
	double	_repeat_lot_mult;	//77
	double	_repeat_progress;	//78
	int		_period;			//79
	double	_deviation;			//80
	double	_stoploss;          //81 ��������
	int		_attemts;			//82
	int		_auto_mm;			//83
	int		_mm_equ;			//84
	double	_basket_hadge_mult; //85
	double	_forward_step_mult;	//86
	double	_delta;				//87
	int		_first_free;		//88
	int		_new_bar;			//89
	int		_free_lvl;			//90
	double  _multf;				//91
	int		_periodf2;			//92
	int		_periodf3;			//93
	int		_buf_len;			//94
	double	_rollback;			//95
	double	_weighthadge;		//96
	//����� ����������
	//double  ask;				//100
	//double  bid;				//101
	double	*open_dd;			//102 ������� �������� �������� �� ������� ���� ��������
	double	*total_lots;		//103 ��������� ����� ����� �� ������� ����
	int		*max_lvl;			//104
	double	*max_dd;			//105
	double	*indicator;			//106
	int		*count_p;			//107

	int		*o_ticket;			//110
	int		*o_type;			//111
	double  *o_lots;			//112
	double	*o_openprice;		//113
	double	*o_slprice;			//114
	double	*o_tpprice;			//115
	double	*indicator2;		//116
	double	prev_indicator;
	int    *intret;				//200 ��� �������

	ORDER	cur_order;
	double  c_weight;
	int     c_index;
	bool    c_all;

	#pragma endregion
};

class DILLER
{
public:

	DILLER		*opposite;
	PARAMETERS	*params;
	int			type;
	ORDERLIST	orders;
	ORDER		*ord_limit;
	ORDER		*ord_stop;
	double		mpo;
	double		mpc;
	int			level;
	double		step_peak;		//��� �������� ����
	double		tp_peak;			//��� ��������� �����������
	ORDER		*last;
	ORDER		*first;
	//int			_stop_new;		//50, 51 ���������� �������� ����� �����
	//int			_stop_avr;		//52, 53 ���������� �������� ����� �������
	int			prev_lvl;		//������� ���������� �����
	double		prev_lots;		//������� ��������� �������� �����
	bool		sorted;
	int			cur_av_lvl;
	int			index; 
	double		_base_lot;


	DILLER(PARAMETERS *_params, int _type)
	{
		params = _params;
		type = _type;
		tp_peak = 0;
		step_peak = 0;
		orders.clear();
		prev_lvl = 0;
		level = 0;
		sorted = false;
		last = first = nullptr;
		index = 0;
	}
	void ResetTick()
	{
		prev_lvl	= level? level: prev_lvl;
		prev_lots	= last? last->lots: prev_lots;
		level		= 0;
		orders.clear();
		*(params->open_dd + type)		= 0.0;
		*(params->total_lots + type)	= 0.0;
		//*(params->count_p + type)		= level;
		ord_limit = ord_stop = nullptr;
		//c_index = 0;
	}

	void AddOrder(ORDER* order)
	{
		orders.push_back(order);
	}
	bool GetOrder(int _num=0)
	{
		if (_num >= level) return false;
		params->cur_order = *orders[_num];
		return true;
	}
	bool CloseByWeight()
	{
		//msg << "opposite index: " << c_index << "\r\n";
		if (params->c_index >= level)  //���� ������� ������ ���, �� ���������� ���������
		{
			//c_index = -1;
			//c_weight = 0.0;
			params->c_weight = -1;
			//msg << "no order opp" << "\r\n";
			return(false);
		}
		if (!GetOrder(params->c_index))
		{
			params->c_index = -1;
			//c_weight = 0.0;
			//msg << "no order" << msg_box;
			return(false);
		}
		double weight = order_weight(params->cur_order.openprice, mpc, params->cur_order.lots);
		if (params->c_weight + weight < 0.0)  //���� ������ ��������� ����� �����
		{
			//msg << "opp_partial close\r\n";
			//msg << "opp_�_weight=" << c_weight * 10000 << "\r\n";
			//msg << "opp_order weight=" << weight * 10000 << "\r\n";
			double min_weight = order_weight(params->cur_order.openprice, mpc, params->lot_min);
			//msg << "opp_min weight=" << min_weight * 10000 << "\r\n";
			if (params->c_weight + min_weight > 0.0)  //���� ����� ��������� ������ ����������� ���
			{
				*(params->o_ticket) = params->cur_order.ticket;
				*(params->o_openprice) = mpc;
				*(params->o_lots) = floor(params->c_weight / abs(min_weight)) * params->lot_step;
				params->c_weight += order_weight(params->cur_order.openprice, mpc, *params->o_lots);
				//msg << "opp_full lots=" << cur_order.lots << "\r\n";
				//msg << "opp_part lots=" << *o_lots << "\r\n";
				//msg << "opp_last c_weight=" << c_weight * 10000 << "\r\n";
				return(true);
			}
			//��������� ��� ��� �����
			//c_index = -1;
			params->c_weight = -1;
			return(false);
		}
		params->c_weight += weight;
		*(params->o_ticket) = params->cur_order.ticket;
		*(params->o_openprice) = mpc;
		*(params->o_lots) = params->cur_order.lots;
		//msg << "opp_end weight2=" << c_weight * 10000 << "\r\n";
		//msg << "opp_close_one" << "\r\n";
		params->c_index++;
		
		return(true);
	}
	double BasketCost()
	{
		double res;
		for (int i=0; i<level; i++)
			res += 1;
		return res;
	}

	#pragma region ������� ��������� �������
	//"������ ����" ��� ����
	double best_price(double a, double b)
	{
		return type? max(a, b): min(a, b);
	}
	//����������� ���� �����������
	double tp(double _open_price, double _tp)
	{
	   return type? _open_price - _tp: _open_price + _tp;
	}
	//����������� ���� ���������
	double sl(double _open_price, double _sl)
	{
	   return type? _open_price + _sl: _open_price - _sl;
	}
	//������ ���� ������ 
	double order_weight(double open_price, double close_price, double _lots)
	{
	   return (type? open_price - close_price: close_price - open_price) * _lots;
	}
	//������ ������ ���� � ������ ���� ��������
	double delta(double low_price, double high_price)
	{
		return type? low_price - high_price: high_price - low_price;
	}
	//����������� ��� �������
	double basket_weight(double _close_price, int _av_lvl=100)
	{
		double weight = 0.0;
		for (int i=0; i < level; i++)
		{
			weight += order_weight(orders[i]->openprice, _close_price, orders[i]->lots);
			if (i+1 >= _av_lvl) return(weight);
		}
		return(weight);
	}

	#pragma endregion
};

class SIMBIOT: public PARAMETERS   //��������� ����������� ��� ������ ������ ���������� ���������
{
public:
	#pragma region ���������� � ������
	//���������� ��������� ������� � ����� ������
	int		index;
	int		old_index;
	int		total_count;
	int		old_count;
	DILLER	*dillers[2];
	DILLER	*curdil;

	ORDER	orders[num_orders];
	ORDER	old_orders[num_orders];
	
	double	base_lot;			//������� ���
	double	equity;
	double	max_equity;
	//double	prev_indicator;
	//move_tp
	double	m_weight;
	double	m_last_weight;
	double	m_total_weight;
	int		m_index;
	int		k;					//���� ��������� ��� ������� � �������
	bool    sorted;
	bool	showend;
	double	profits[50];
	double	p_high, p_low, p_buy, p_sell;

	//���������� ��� ���������� �����������
	double	*closes;
	double	*highs;
	double  *lows;
	double	tmAvr;
	double	tmBuffer[max_bars];
	double	up_ind;
	double	dn_ind;
	double  wu, wd;
	double  prev_wu, prev_wd;
	double	wuBuffer[max_bars];
	double	wdBuffer[max_bars];
	double	maBuffer[3];
	int		counted;
	int		bars;
	double  tmup, tmdown;
	bool	first_calc;
	//bool	prev


	#pragma endregion
public:
	SIMBIOT()
	{
		dillers[0] = new DILLER(this, 0);
		dillers[1] = new DILLER(this, 1);
		dillers[0]->opposite = dillers[1];  //����� �������� ���� �� �����
		dillers[1]->opposite = dillers[0];
		curdil = dillers[0];
		total_count = 0;
		p_high = 0;
		p_low = 1000000;
		p_buy = 0;
		p_sell = 0;
		counted = 0;

		
		k=0;
		m_index = 0;
		c_index = 0;
		c_all = false;
		showend = true;
		
		//msg << "SIMBIOT OK" << msg_box;
	}
	void PostInit()
	{
		//bp = true;
		//msg << "tp_mult=" << _tp_mult << "\r\n";
		for (int i=0; i<50; i++)
		{
			profits[i] = _takeprofit * pow(_pips_mult, i);
			//msg << "steps[" << i << "]=" << steps[i] << ", tps = " << tps[i] << "\r\n";
		}
		*indicator = 0;
		prev_indicator = -50;
		first_calc =  true;
		dillers[0]->_base_lot = _buy_lot;
		dillers[1]->_base_lot = _sell_lot;
		curdil->cur_av_lvl = _av_lvl;
		//msg << msg_box;
		//bp = false;
		//msg << "trailing step: " << _tr_step << "\r\n";
		//msg << "chk_new min" << min_sl_tp << msg_box;
	}
	void Sort()
	{
		if (sorted) return;
		if (dillers[0]->level = dillers[0]->orders.size())
		{
			std::sort(dillers[0]->orders.begin(), dillers[0]->orders.end(), [](const ORDER* a, const ORDER* b) {return a->openprice > b->openprice;});
			dillers[0]->first = dillers[0]->orders[0];
			dillers[0]->last = dillers[0]->orders.back();
		}
		else
			dillers[0]->first = dillers[0]->last = nullptr;

		if (dillers[1]->level = dillers[1]->orders.size())
		{
			std::sort(dillers[1]->orders.begin(), dillers[1]->orders.end(), [](const ORDER* a, const ORDER* b) {return a->openprice < b->openprice;});
			dillers[1]->first = dillers[1]->orders[0];
			dillers[1]->last = dillers[1]->orders.back();
		}
		else
			dillers[1]->first = dillers[1]->last = nullptr;
		curdil = dillers[0];
		sorted = true;
		*count_p = dillers[0]->level;
		*(count_p+1) = dillers[1]->level;
	}


	void calc_first_lot()
	{
		curdil->cur_av_lvl = _av_lvl;		//��������������� ������� ������� ���������� �� ��������
		if (_auto_mm > 0)				//���� ������� ��������������
			base_lot = normlot(floor(equity/_mm_equ) * curdil->_base_lot);
		else
			base_lot = curdil->_base_lot;
		*o_lots = base_lot;
		if (curdil->opposite->last)
			*o_lots = max(*o_lots, curdil->opposite->last->lots * _lot_hadge_mult);		//������������ ������� ������������
		if (_weighthadge)
			*o_lots = max(*o_lots, (_takeprofit * base_lot - curdil->opposite->basket_weight(*o_tpprice, 100)*_weighthadge) / _takeprofit);
		*o_lots = max(*o_lots, *(total_lots + curdil->opposite->type) * _basket_hadge_mult);
		//msg << "hadge(" << _hadge_mult << "): " << *o_lots << "\r\n";
		*o_lots = max(*o_lots, curdil->prev_lots * _regres_mult);		//���������
		//msg << "regres: " << *o_lots << msg_box;
		*o_lots = min(*o_lots, _maxlot);
	}
	bool calc_first()
	{
		//ShowInfo("calc_first", "start");
		/*if (_tr_step > 0 && curdil->delta(curdil->step_peak, curdil->mpo) < _tr_step)
		{
			curdil->step_peak = curdil->best_price(curdil->step_peak, curdil->mpo);
			ShowInfo("calc_first trailing peak", curdil->step_peak);
			ShowInfo("calc_first delta", curdil->delta(curdil->step_peak, curdil->mpo));
			return(false);
		}*/
		*o_openprice = curdil->mpo;
		//if (curdil->level == 0)
		//	*o_tpprice = curdil->tp(*o_openprice, _takeprofit);
		//else
			*o_tpprice = curdil->tp(*o_openprice, _takeprofit*10);
		//ShowInfo("calc_first: _takeprofit", _takeprofit);
		//ShowInfo("calc_first: tp_price",  curdil->tp(*o_openprice, _takeprofit));
		*o_type = curdil->type;
		
		*o_slprice = curdil->sl(*o_openprice, _stoploss);
		*intret = check_new();
		if (*intret > 0)
		{
			ShowInfo("*** calc_first check error", *intret);
			return(false);}
		*intret = 1;
		return(true);
	}
	bool calc_forward()
	{
		*o_openprice = curdil->tp(curdil->first->openprice, _step * _forward_step_mult);
		double d = curdil->delta(curdil->mpo, *o_openprice);
		/*msg << "try calc forward\r\n";
		msg << "mpo: " << mpo[type] << "\r\n";
		msg << "first_price: " << first_price[type] << "\r\n";
		msg << "openprice: " << *o_openprice << "\r\n";
		msg << "delta: " << d << msg_box;*/
		if (d > min_sl_tp)   //���� ����� ��������� �������
			*o_type = curdil->type + 4;
		else if (d <= 0)	//���� ������ �������, �� ����� �� �����
		{
			*o_openprice = curdil->mpo;
			*o_type = curdil->type;
		}
		else				//���� �� ����� ��������� �� ������� �� �� �����
			return(false);
		calc_first_lot();
		*o_tpprice	= curdil->tp(*o_openprice, _takeprofit);
		*o_slprice	= curdil->sl(*o_openprice, _stoploss); 
		*intret = check_new();
		if (*intret > 0)
		{
			ShowInfo("*** calc_forward check error", *intret);
			return(false);}
		return(true);
	}
	bool calc_next()
	{
		*o_openprice  = curdil->sl(curdil->last->openprice, _step);
		double d = curdil->delta(*o_openprice, curdil->mpo);  //���������� �� �������� ������, ���� �������������, �� ��������, ���� ������� �� �����
		//ShowInfo("calc_next fixed step, d", d);
		if (d <= 0)  //���������� �� �����
		{
			*o_openprice = curdil->mpo;
			*o_type = curdil->type;
		}
		else   //���� �� ����� ��������� �� ������� �� �� �����
		{
			curdil->step_peak = 0;
			return(false);
		}
		*o_slprice = curdil->sl(*o_openprice, _stoploss);
		if (curdil->last->lots >= _av_lot)
		{
			if (curdil->cur_av_lvl == _av_lvl && curdil->level < _av_lvl)
				curdil->cur_av_lvl = max(1, curdil->level);
			else
				curdil->cur_av_lvl = max(1, curdil->cur_av_lvl-1);
		}
		*o_tpprice = curdil->tp(*o_openprice, _takeprofit*10);
		double nextProfit = profits[curdil->level-1];
		*o_lots = (nextProfit * base_lot - curdil->order_weight(curdil->first->openprice, *o_tpprice, lot_min)) / _takeprofit;
		*o_lots = max(*o_lots, (nextProfit * base_lot - curdil->basket_weight(*o_tpprice) * _multf) / _takeprofit);
		if (_safe_copy)
			*o_lots = max(*o_lots, base_lot);
		*o_lots = min(*o_lots, _maxlot);
		//*o_lots = base_lot;
		*intret = check_new();
		if (*intret > 0)
		{
			ShowInfo("*** calc_next check error", *intret);
			return(false);}
		*intret = curdil->level + 1;
		return(true);
	}
	bool calc_oposite()
	{
		//ShowInfo("calc_oposite", "start");
		//if (curdil->opposite->level > _op_av_lvl && curdil->opposite->GetOrder())
		if (curdil->opposite->GetOrder() && abs(cur_order.openprice - curdil->mpo) > _step * _op_av_lvl)

		{
			//msg << "try calc oposite" << msg_box;
			double tp_price = curdil->tp(curdil->mpo, _takeprofit);
			double oplots = (_takeprofit * base_lot - curdil->opposite->order_weight(cur_order.openprice, tp_price, lot_min))/_takeprofit;
			if (oplots > _maxlot)  //���� ��� ��������� ��������, �� ������� �� ���������
				return(false);
			*o_lots = max(*o_lots, oplots);
			*o_lots = min(*o_lots, _maxlot);
			*o_ticket = cur_order.ticket;
			*o_openprice = cur_order.openprice;
			*o_slprice = tp_price;
			*o_tpprice = cur_order.tpprice;
			*intret = check_mod();
			if (*intret > 0)
				return(false);
			return(false);
		}
		return(false);
	}
	bool move_tp()
	{
		ShowInfo("move_tp", "start");
		if (curdil->level < 2)   //���� � ����� ���� �����, �� ������ � �������
		{
			ShowInfo("move_tp", "nothing to move");
			return(false);}
		if (curdil->order_weight(curdil->first->openprice, curdil->mpc, 1) > 0)
			return(false);


		double last_tp = curdil->last->tpprice;
		if (m_index == 0)
		{
			m_weight = 0.0;
			m_last_weight = curdil->order_weight(curdil->last->openprice, last_tp, curdil->last->lots);
			m_total_weight = curdil->basket_weight(last_tp);
		}
		ShowInfo("move_tp", "m_index==0");
		if (m_index < curdil->level-1)
		{
			if (!curdil->GetOrder(m_index))
			{
				m_index = 0;
				ShowInfo("move_tp", "no order");
				return(false);
			}
			if (m_total_weight < 0.0)
			{
				m_weight += curdil->order_weight(cur_order.openprice, last_tp, cur_order.lots);
				if (m_last_weight + m_weight <= 0.0)
				{
					m_index = 0;
					ShowInfo("move_tp", "cant average");
					return(false);
				}
			}
			if (abs(cur_order.tpprice - last_tp) > point)
			{
				m_index++;
				*o_ticket = cur_order.ticket;
				*o_openprice = cur_order.openprice;
				*o_tpprice = last_tp;
				*o_slprice = cur_order.slprice;
				*intret = check_mod();
				if (*intret > 0)
				{
					ShowInfo("move_tp check_mod error", *intret, true);
					return(false);}
				ShowInfo("move_tp", "mod order");
				return(true);
			}
		}
		m_index = 0;
		return(false);
	}
	bool signal1()
	{
		
		if (_first_free && curdil->level == 0)  //������ ����� ��������� � ����� ������
			return true;
		/*
		if (curdil->type)
		{
			if (*indicator > - _rsi_delta && *indicator2 < -_rsi_delta)
				return true;
		}
		else
		{
			if (*indicator < (_rsi_delta - 100) && *indicator2 > (_rsi_delta - 100))
				return true;
		}
		*/

		/*if (curdil->type)
		{
			if (*indicator >= -_rsi_delta)
				prev_indicator = max(prev_indicator, *indicator);
			if (prev_indicator >= - _rsi_delta && *indicator < -_rsi_delta2)
			{
				prev_indicator = -50;
				bp = true;
				return true;}
		}
		else
			if (*indicator <= (_rsi_delta - 100))
				prev_indicator = min(prev_indicator, *indicator);
			if (prev_indicator <= (_rsi_delta - 100) && *indicator > (_rsi_delta2 - 100))
			{
				prev_indicator = -50;
				bp = true;
				return true;}*/
		/*if (dillers[0]->mpc - *indicator > _rsi_delta)
			prev_indicator = 1;
		if (*indicator - dillers[0]->mpc > _rsi_delta)
			prev_indicator = -1;

		if (curdil->type)
		{

			if (prev_indicator > 0 && curdil->mpo < *indicator)
			{
				prev_indicator = 0;
				return true;
			}
		}
		else
		{
			if (prev_indicator < 0 && curdil->mpc > *indicator)
			{
				prev_indicator = 0;
				return true;
			}
		}*/
		/*if (curdil->type)
		{
			if (prev_indicator > 0 && *indicator < *indicator2 && *indicator > 100 - _rsi_delta2 && *indicator < 100 - _rsi_delta)
			{
				return true;
			}
		}
		else
		{
			if (prev_indicator < 0 && *indicator > *indicator2 && *indicator < _rsi_delta2 && *indicator > _rsi_delta)
			{
				return true;
			}
		}*/
		//if (prev_indicator == *indicator)
			//return false;
		/*
		if (curdil->type)
		{
			if (*indicator > _rsi_delta && *indicator2 <= _rsi_delta)
				return true;
		} else {
			if (*indicator < -_rsi_delta && *indicator2 >= -_rsi_delta)
				return true;
		}*/
		if (curdil->type)
		{
			if (*indicator > 0)
				return true;
		}
		else
		{
			if (*indicator < 0)
				return true;
		}



		return false;
	}
	bool signal()
	{
		if (_first_free && curdil->level == 0)  //������ ����� ��������� � ����� ������
			return true;
		//*indicator = 0;
		//*indicator2 = 0;
		//bp = true;
		if (curdil->level >= _free_lvl)
			return true;
		//bool fract = tmBuffer[_period] < tmup && tmup > tmBuffer[1];
		//double d;
		if (curdil->type) //�������
		{
			//d= _deviation - max((((curdil->level)?curdil->mpo - curdil->last->openprice: 0) - _step) * _multf, 0);
			//*indicator2 = d;
			if (curdil->step_peak)
			{
				if ((_periodf3 && maBuffer[0] < maBuffer[1]) ||
					(_periodf3 == 0 && curdil->step_peak - curdil->mpo >= _rollback) ||
					(_periodf3 == 0  && _rollback == 0))
				{
					curdil->step_peak = 0;
					return true;
				}
				else
					curdil->step_peak = max(curdil->step_peak, curdil->mpo);

			}
			else if (curdil->mpo > up_ind)
			{
				//msg << "start trall sell" << msg_box;
				curdil->step_peak = curdil->mpo;
			}
			/*if (fract && !prev_indicator)// || (tmBuffer[_period] < tmup && tmup > tmBuffer[1]))
			{
				prev_indicator = fract;
				*indicator2 = 1;
				//_step = up_ind - tmBuffer[0];
				//_takeprofit = _step * 1.5;
				return true;
			}*/
			/*if (*indicator2 > 0)
			{
				_step = max(0.001, *indicator2 * 0.4);
			_takeprofit = max(0.0015, *indicator2 * 0.6);
				return true;}*/
				

			/*
			if (*indicator2 == 0)
				return false;
			//msg >> "ind: " >> *indicator
			_step = max(0.001, *indicator2 * 0.4);
			_takeprofit = max(0.0015, *indicator2 * 0.6);*/
		}
		else //�������
		{
			//d= _deviation - max((((curdil->level)?curdil->last->openprice - curdil->mpo: 0) - _step) * _multf, 0);
			//*indicator = d;
			if (curdil->step_peak)
			{
				if ((_periodf3 && maBuffer[0] > maBuffer[1]) ||
					(_periodf3 == 0 && curdil->mpo - curdil->step_peak >= _rollback) ||
					(_periodf3 == 0  && _rollback == 0))
				{
					curdil->step_peak = 0;
					return true;
				}
				else
					curdil->step_peak = min(curdil->step_peak, curdil->mpo);

			}
			else if (curdil->mpc < dn_ind)
			{
				//msg << "start trall buy" << msg_box;
				curdil->step_peak = curdil->mpo;
			}
			/*if (!fract && prev_indicator)//(tmBuffer[_period] > tmdown && tmdown < tmBuffer[1]))
			{
				prev_indicator = fract;
				*indicator = 1;
				//_step = tmBuffer[0] - dn_ind;
				//_takeprofit = _step * 1.5;
				return true;
			}*/
			/*if (*indicator >0)
			{
				_step = max(0.001, *indicator * 0.4);
			_takeprofit = max(0.0015, *indicator * 0.6);
			return true;}*/
			/*
			if (*indicator == 0)
				return false;
			_step = max(0.001, *indicator * 0.4);
			_takeprofit = max(0.0015, *indicator * 0.6);*/
		}
		//prev_indicator = fract;
		return false;
	}
	void calc_indicator()
	{
		__try
		{
		double main_avr;
		double diffup;
		double diffdn;
		//msg << "buf_len: " << _buf_len << "\r\n";
		closes[_buf_len] = dillers[0]->mpc;
		highs[_buf_len] = max(dillers[0]->mpc, highs[_buf_len]);
		lows[_buf_len] = min(dillers[0]->mpc, lows[_buf_len]);

		int start = min(bars - counted, _periodf2);
		if (start == _periodf2)
		{
			prev_wd = prev_wu = _delta*_delta;
		}
		//msg << "start: " << start << "\r\n";
		for (int i=start; i>=0; i--)
		{
			main_avr = GetMAW(_period, i);
			diffup = max(highs[_buf_len-i]-main_avr, _delta);
			diffdn = max(main_avr-lows[_buf_len-i], _delta);
			diffup *= diffup;
			diffdn *= diffdn;
			wu = (prev_wu*(_periodf2-1)+diffup)/_periodf2;
			wd = (prev_wd*(_periodf2-1)+diffdn)/_periodf2;
			if (i>0)
			{
				prev_wu = wu;
				prev_wd = wd;
			}
		}
		//msg << "end\r\n";
		counted = bars;
		up_ind = main_avr + _deviation * sqrt(wu);
		dn_ind = main_avr - _deviation * sqrt(wd);

		if (_periodf3)
			for (int i = 1; i >=0; i--)
			{
				maBuffer[i] = GetMAW(_periodf3, i);
			}
		//*indicator2 = up_ind;
		//msg << msg_box;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bp = true;
			ShowInfo("calc_indicator ERROR", GetExceptionCode(), true);
		};
	}
	double GetMAW(int period, int shift)
	{
		int j, k;
		double sum  = (period + 1) * closes[_buf_len - shift];
		double sumw = period + 1;
		for(j=1, k=period; j<=period; j++, k--)
		{
			sum  += k*closes[_buf_len - (shift + j)];
			sumw += k;
		}
		return sum/sumw;
	}



	void refresh_prices(double *_closes, double *_highs, double *_lows, int _bars)
	{
		__try
		{
		closes = _closes;
		highs = _highs;
		lows = _lows;
		bars = _bars;
		calc_indicator();
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bp = true;
			ShowInfo("refresh_prices ERROR", GetExceptionCode(), true);
		};

	}
	bool close_profit()
	{
		if (curdil->level > c_index && curdil->basket_weight(curdil->mpc)>0)// first->profit > 0)
		{

	
			//msg << "I have profit!" << msg_box;
			if (curdil->type == OP_SELL && *indicator > 0)  //������� �������� sell
			{
				//msg << "SELL must be closed" << msg_box;
				*o_ticket = curdil->orders[c_index]->ticket;
				*o_lots = curdil->orders[c_index]->lots;
				*o_openprice = curdil->mpc;
				c_index++;
				return true;
			}
			if (curdil->type == OP_BUY && *indicator2 > 0)  //������� �������� buy
			{
				//bp = true;
				//msg << "BUY must be closed" << msg_box;
				*o_ticket = curdil->orders[c_index]->ticket;
				*o_lots = curdil->orders[c_index]->lots;
				*o_openprice = curdil->mpc;
				c_index++;
				return true;
			}
		}
		c_index = 0;
		return false;
	}
	bool close_profit2()
	{
		
		//msg << "index=" << c_index << "\r\n";
		if (curdil->delta(curdil->last->openprice, curdil->mpc) < _takeprofit)
			return false;
		//bp = true;
		if (c_index < 0)  //�������������
		{
			c_index = 0;
			c_weight = curdil->basket_weight(curdil->mpc);
			//msg << "c_weight: " << c_weight*100000 <<  "\r\n";
			if (c_weight > 0.0)   //���� ��� ���� ����� �������������, �� ��������� ��� �����
				c_all = true;
			else
			{
				c_weight = curdil->order_weight(curdil->last->openprice, curdil->mpc, curdil->last->lots - lot_min);//curdil->_base_lot);
				c_all = false;
			}
		}
		

		//msg << "close profit, type: " << curdil->type <<  "\r\n";
		//msg << "close all: " << c_all << "\r\n";
		//msg << "c_weight: " << c_weight*100000 << msg_box;

		if (c_all && c_weight>0 && curdil->opposite->level >= _op_av_lvl)   //�������� ����������
		{
			//msg << "opposite average: " << c_weight*100000 << msg_box;//"\r\n";

			if (curdil->opposite->CloseByWeight())
				return(true);
			c_index = 0;
		}	
		//msg << "basket weight=" << basket_weight(mpc[type]) * 10000 << "\r\n";
		//msg << "c_weight=" << c_weight * 10000 << "\r\n";
		//msg << "c_all=" << c_all << "\r\n";
		if (!curdil->GetOrder(c_index))
		{
			c_index = -1;
			c_all = false;
			//c_weight = 0.0;
			//msg << "no order" << msg_box;
			return(false);
		}
		if (c_all)  //���� ��������� ��� �����
		{
			*o_ticket = cur_order.ticket;
			*o_openprice = curdil->mpc;
			*o_lots = cur_order.lots;
			c_index++;
			if (c_index >= curdil->level)
			{
				c_index = -1;
				k=100;
			}
			//msg << "close_all" << msg_box;
			return(true);
		}
		double weight = curdil->order_weight(cur_order.openprice, curdil->mpc, cur_order.lots);
		//msg << "weight=" << weight * 10000 << "\r\n";
		if (c_weight + weight < 0.0)  //���� ������ ��������� ����� �����
		{
			//msg << "partial close\r\n";
			//msg << "�_weight=" << c_weight * 10000 << "\r\n";
			//msg << "order weight=" << weight * 10000 << "\r\n";
			double min_weight = curdil->order_weight(cur_order.openprice, curdil->mpc, lot_min);
			//msg << "min weight=" << min_weight * 10000 << "\r\n";
			if (c_weight + min_weight > 0.0)  //���� ����� ��������� ������ ����������� ���
			{
				*o_ticket = cur_order.ticket;
				*o_openprice = curdil->mpc;
				*o_lots = floor(c_weight / abs(min_weight)) * lot_step;
				c_weight += curdil->order_weight(cur_order.openprice, curdil->mpc, *o_lots);
				//msg << "full lots=" << cur_order.lots << "\r\n";
				//msg << "part lots=" << *o_lots << "\r\n";
				//msg << "last c_weight=" << c_weight * 10000 << "\r\n" << msg_box;
				return(true);
			}
			//��������� ��� ��� �����, ��������� ������� ���������� �����
			c_index = -1;
			*o_ticket = curdil->last->ticket;
			*o_openprice = curdil->mpc;
			*o_lots = curdil->last->lots;
			k=100;
			return(true);
		}
		
		c_weight += weight;
		//msg << "end weight2=" << c_weight * 10000 << "\r\n";
		//msg << "close_one" << msg_box;
		c_index++;
		if (c_index >= curdil->level)
		{
			c_index = -1;
			k=100;
		}
		*o_ticket = cur_order.ticket;
		*o_openprice = curdil->mpc;
		*o_lots = cur_order.lots;
		return(true);
	}


	//����������� ����
	double norm(double value)
	{
		return(floor(value / point + 0.5) * point);
	}
	//����������� ���
	double normlot(double value)
	{
	   int steps = int(ceil(value / lot_step));
	   value = steps * lot_step;
	   value = max(value, lot_min);
	   return min(value, lot_max);
	}
	//�������� ������ ��������� ��� �����������
	bool check_sl(int _type, double low_price, double high_price)   //������� high low ��� �������
	{
		return(!(low_price && high_price && dillers[_type % 2]->delta(low_price, high_price) < min_sl_tp));
	}
	//�������� ������ ���������
	bool check_freeze(double pr1, double pr2)
	{
		return(abs(pr1-pr2) > freeze);
	}
	//�������� �� ������������ ���������� ��� �������� ������
	int check_new()
	{
		int t = *o_type % 2;
		if (*o_type < 2 && //��� ������������ ����������
			(!check_sl(t, *o_slprice, dillers[t]->mpc) || 
			!check_sl(t, dillers[t]->mpc, *o_tpprice)))
		{
			ShowInfo("chk_new tp", *o_tpprice);
			ShowInfo("chk_new sl", *o_tpprice);
			ShowInfo("chk_new min", min_sl_tp);
				return(201);}
		if (*o_type > 1 &&  //��� ���������� �������
			(!check_sl(t, *o_openprice, *o_tpprice) ||
			!check_sl(t, *o_slprice, *o_openprice)))
			return(201);
		if ((*o_type == OP_BUYLIMIT || *o_type == OP_SELLLIMIT) &&
			!check_sl(t, *o_openprice, dillers[t]->mpo))
			return(202);
		if ((*o_type == OP_BUYSTOP || *o_type == OP_SELLSTOP) &&
			!check_sl(t, dillers[t]->mpo, *o_openprice))
			return(202);
		*o_lots			= normlot(*o_lots);
		*o_openprice	= norm(*o_openprice);
		*o_tpprice		= norm(*o_tpprice);
		*o_slprice		= norm(*o_slprice);
		return(0);
	}
	//�������� �� ������������ ���������� ��� ����������� ������
	int check_mod()
	{
		if (get_order(*o_ticket) == 0)
			return(200);
		*o_openprice = norm(*o_openprice);
		*o_tpprice	= norm(*o_tpprice);
		*o_slprice	= norm(*o_slprice);
		int _type = cur_order.type;
		double _mpc = dillers[_type]->mpc;
		double _mpo = dillers[_type]->mpo;
		if (_type < 2 &&   //��� �������� �������
			(!check_freeze(cur_order.slprice, _mpc) ||
			!check_freeze(cur_order.tpprice, _mpc) ||
			!check_sl(_type, *o_slprice, _mpc) || 
			!check_sl(_type, _mpc, *o_tpprice)))
			return(201);
		if (_type > 1 &&  //��� ���������� �������
			(!check_freeze(cur_order.openprice, _mpo) ||
			!check_sl(_type, *o_slprice, *o_openprice) || 
			!check_sl(_type, *o_openprice, *o_tpprice)))
			return(201);
		if ((_type == OP_BUYLIMIT || _type == OP_SELLLIMIT) &&
			!check_sl(_type, cur_order.openprice, _mpo))
			return(202);
		if ((_type == OP_BUYSTOP || _type == OP_SELLSTOP) &&
			!check_sl(_type, _mpo, cur_order.openprice))
			return(202);
		return(0);
	}
	#pragma region ������ ����������� ��������� �������
	//3.1 ������������� ����� ���������� �������, ��������� ���� ����������� ������ (������ �������� ���������� �� MQL), ������ ����������� ������������ � MQL ��������� �� ���������� �����
	void refresh_init(double ask, double bid, double equity_)   //+
	{
		__try
		{
			sorted = false;
			equity = equity_;
			dillers[0]->mpo = dillers[1]->mpc = ask;
			dillers[1]->mpo = dillers[0]->mpc = bid;
			//mpo[0] = mpo[2] = mpo[4] = mpc[1] = mpc[3] = mpc[5] = ask;
			//mpo[1] = mpo[3] = mpo[5] = mpc[0] = mpc[2] = mpc[4] = bid;
			//max_equity = max(max_equity, equity);
			//*max_dd = max(max_equity - equity, *max_dd);
			index = 0;  //�������� ����� ������� �������
			//ShowInfo("refresh_init", "bp1");
			//prev_indicator = (*indicator==10)? prev_indicator : *indicator;
			//*indicator = 10;
			if (!is_optimization)   //���� �� �����������
			{
				old_index = 0;
				memcpy(old_orders, orders, sizeof(orders));  //������ ������� �������, ������ ������
				old_count = total_count;
			}
			//ShowInfo("refresh_init", "bp2");
			dillers[0]->ResetTick();    //���������� ������������� ���������� �� �����
			dillers[1]->ResetTick();
			k = 0;			//� ����� ����� ���������� �������� ��������
			c_index = -1;
			if (!_new_bar)
				calc_indicator();
			//msg << "refresh init done" << msg_box;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bp = true;
			msg << "refresh_init ERROR: " << GetExceptionCode() << msg_box;
		};
	}
	//3.1 ��������� ����� ����� � ����� ����� �������, � ������� ���������� ��� ���������
	int refresh_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit=0)
	{
		__try
		{
			//msg << "refresh start" << msg_box;
			orders[index].ticket = _ticket;
			orders[index].type = _type;
			orders[index].lots = _lots;
			orders[index].openprice = _openprice;
			orders[index].tpprice = _tp;
			orders[index].slprice = _sl;
			//orders[index].checked = false;
			if (_type<2)
			{
				orders[index].profit = _profit;
				*(open_dd+_type)        += _profit;
				*(total_lots+_type)     += _lots;
				dillers[_type]->orders.push_back(&orders[index]);
			}
			else if (_type < 4)
				dillers[_type % 2]->ord_limit = &orders[index];
			else
				dillers[_type % 2]->ord_stop = &orders[index];
			total_count = ++index;
			//msg << "refresh order done " << index << msg_box;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bp = true;
			msg << "refresh_order ERROR: " << GetExceptionCode() << msg_box;
		};

		return(0);
	}
	//������ ������ �������� �������, ���� ��� ����
	int getclosed()
	{
		bool _flag;
		while (old_index < old_count)
		{
			if (old_orders[old_index].type > OP_SELL)   //���������� �� �������� ������
			{
				old_index++;
				continue; 
			}
			_flag = false;
			for (int i=0; i<total_count; i++)
			{
				if (old_orders[old_index].ticket == orders[i].ticket)
				{
					_flag = true;
					break;
				}
			}
			old_index++;
			if (!_flag)
				return(old_orders[old_index-1].ticket);
		}
		return(0);
	}
	//������ ����� �� ��������� �� ������
	bool get_order(int _ticket)
	{
		for (int i=0; i<total_count; i++)
			if (orders[i].ticket == _ticket)
			{
				cur_order = orders[i];
				return(true);
			}
		return(false);  //�� �����
	}

	//		ShowInfo("calc_first_lot", "BreakPoint 1", true);
	//} catch(std::exception e) {ShowInfo("ERROR: calc_first_lot", e.what(), true);};
	template <class T> void ShowInfo(char text[], T value, bool show=false)
	{
#if debug 
		//bp = true;
		if (!is_visual)
			return;
		if (showend)
		{
			msg << "Ask: " << dillers[0]->mpo << "   Bid: " << dillers[1]->mpo << "\r\n";
			LPCSTR s = (curdil->type)? " Sell": "  Buy";
			msg << "Type: " << curdil->type << s << " sorted=" << sorted << "\r\n";
			msg << "Count Buy: " << dillers[0]->level << " (" << dillers[0]->orders.size() << "), Sell: " << dillers[1]->level << "\r\n";
			msg << "k=" << k << " total: " << total_count << "\r\n";
			msg << "o_type=" << *o_type << ", o_lot=" << *o_lots << "\r\n";
			msg << "o_price=" << *o_openprice << ", o_tp=" << *o_tpprice << "\r\n";
			showend = false;
		}

		msg << text << ": " << value << "\r\n";
		if (show)
		{
			showend=true;
			msg << msg_box;
			//bp = false;
		}
#endif
	}

	#pragma endregion

	int getjob()
	{
		__try
		{
		//ShowInfo("======== get_job", "start ========");
		while(true)		//����� ������ ��� �� �������� � MQL
		{
			//ShowInfo("----- getjob work", k);
			switch(k)
			{
			case 0: //����� ������ ����
				Sort();
				if (!curdil->level)		//���� ��� ������� � �����
				{
					if (!_stop_new[curdil->type] && (signal() || curdil->opposite->level >= _forward_lvl))	//���� �� ��������� �������� ����� ����� � ���� ������
					{  //��������� ������ �����
						calc_first_lot();
						k=1;
						if (calc_oposite())		//������������ �������� ����������
							return(2);
						break;
					}
					k=2; break;
				}
				else if (!_stop_avr[curdil->type] && curdil->level < _max_grid_lvl)   //���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
				{
					k=4;
					if (curdil->opposite->level >= _forward_lvl && !curdil->ord_stop && calc_forward())   //���� ��������� �������� � ��� ����� ���������
						return(1);
					break;   //����������� �������� �����
				}
				else  //���� ���� ������, �� �� ��������� ��������� -> ������� �������
					k=3;
				break;
			case 1: //��������� ������ �����
				k=2;
				if (calc_first())
				{
					//ShowInfo("neworder first", *o_lots);
					return(1);}		//������� ������ �����
				break;
			case 2: //�������� ���������������� ���� ������
				k=3;
				if (curdil->opposite->ord_stop)
				{
					*o_ticket = curdil->opposite->ord_stop->ticket;
					//ShowInfo("delorder stop opposite", *o_ticket);
					return(3);  //������� ���� �����
				}
				break;
			case 3:	//�������� ������ ���� ������
				k=5;
				if (curdil->ord_stop)
				{
					*o_ticket = curdil->ord_stop->ticket;
					//ShowInfo("delorder stop", *o_ticket);
					return(3);	//������� ���� �����
				}
				break;
			case 4:  //������� �����������, ��������� ����������� ������
				//if (move_tp())	//���� ���� ��� �������, �������
					//return(2);
				k=5;
				if (signal() && calc_next()) //���� ����� ��������� ������� ��� �� �����, ����������
					return(1);
				break;
			case 5:
				if (close_profit2())
					return(4);
				k=100;
				break;

			case 100: //���������� ������, ��������� ������ ��� ��������� ���������
				k = 0;  //��������� ������ ������
				//ShowInfo("k100 start", "");
				if (curdil->type == 0)
					{curdil = dillers[1];
					//ShowInfo("======== Start Sell ========", "");
					break;}	//��� ������� ������ ��� �������
				curdil = dillers[0];
				//ShowInfo("======== get_job", "end ========", true);
				bp = false;
				prev_indicator = *indicator;//(*indicator > *indicator2)? 1: -1;
				return(0);	//������������ �������� � ��������� ��������
			}
		
		}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			//bp = true;
			ShowInfo("get_job ERROR", GetExceptionCode(), true);
		};
		return(0);
	}
	//����������, ���� �������� ��, ��� �� ��������� �� ��������
	/*~SIMBIOT()
	{
		delete dillers[0];
		delete dillers[1];
	}*/
};

SIMBIOT *pool[Pool_Size];

#pragma region ��������� � MQL
//���� ��������� ������ � ���� � ������� ����� ��������� ��������
_DLLAPI int __stdcall c_init()    
{
	m.lock();
	if (first_run)
	{
		memset(pool, 0, Pool_Size);
		first_run = false;
	}


	for (int h=0; h<Pool_Size; h++)
	{
		if (pool[h] == nullptr)
		{
			//bp = true;
			//msg << "Pool position: " << h << msg_box;
			pool[h] = new SIMBIOT();
			m.unlock();
			return(h);
		}
	}
	m.unlock();
	bp = true;
	msg << "Pool position not found" << msg_box;
	return(-1);
}
//����������� ������ � ������ � ����
_DLLAPI void __stdcall c_deinit(int h)   
{
	m.lock();
	delete pool[h];
	pool[h] = nullptr;
	m.unlock();
}
//��������� ���� ���������, ���������� ������ ������������ ��������
_DLLAPI int __stdcall c_getjob(int h)
{
	return(pool[h]->getjob());
}
//������ ������� ���������� ������ (��� ����������� ��������������� �������)
_DLLAPI int __stdcall c_getdpi()
{
	__try
	{
		HDC hDC = ::GetDC(NULL); 
		int nDPI = ::GetDeviceCaps(hDC, LOGPIXELSX); 
		ReleaseDC(NULL, hDC);
		return(nDPI);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		msg << "c_getdpi ERROR: " << GetExceptionCode() << msg_box;
	};
	return(1);
}
//�������� �������� ���������
_DLLAPI void __stdcall c_setint(int h, int index, int value)
{
	__try
	{
		switch(index)
		{
			case 2:		pool[h]->digits = value;				//2 ���������� ���������� ������ ��� �����������
			case 9:		pool[h]->is_optimization = value;		//9 ���� �����������
			case 10:	pool[h]->is_visual = value;				//10 ���� ����������� ������
			case 11:	pool[h]->is_testing = value;			//11 ���� ������������
			case 50:	pool[h]->_stop_new[0] = value;			//50, 51 ���������� �������� ����� �����
			case 51:	pool[h]->_stop_new[1] = value;			//50, 51 ���������� �������� ����� �����
			case 52:	pool[h]->_stop_avr[0] = value;			//52, 53 ���������� �������� ����� �������
			case 53:	pool[h]->_stop_avr[1] = value;			//52, 53 ���������� �������� ����� �������
			case 54:	pool[h]->_max_grid_lvl = value;			//54 ������������ ������� �����
			case 59:	pool[h]->_forward_lvl = value;			//59 � ������ ������ ���������� ���������� ������
			case 64:	pool[h]->_av_lvl = value;				//64 ������� ����������
			case 66:	pool[h]->_op_av_lvl = value;			//66 ������� ������ ���������������� ����������
			case 68:	pool[h]->_safe_copy = value;			//68 ����� ������ ������� � �������� ���� � �� � ����������
			case 73:	pool[h]->_trend_lvl = value;			//73
			case 76:	pool[h]->_repeat_lvl = value;			//76
			case 79:	pool[h]->_period = value;				//79
			case 82:	pool[h]->_attemts = value;				//82
			case 83:	pool[h]->_auto_mm = value;				//83
			case 84:	pool[h]->_mm_equ = value;				//84
			case 88:	pool[h]->_first_free = value; break;	//88
			case 89:	pool[h]->_new_bar = value; break;		//89
			case 90:	pool[h]->_free_lvl = value; break;		//90
			case 92:	pool[h]->_periodf2 = value; break;		//92
			case 93:	pool[h]->_periodf3 = value; break;		//93
			case 94:	pool[h]->_buf_len = value; break;		//94
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bp = true;
		msg << "c_setint ERROR: " << GetExceptionCode() << msg_box;
	};
}
_DLLAPI void __stdcall c_setdouble(int h, int index, double value)
{
	__try
	{
		switch(index)
		{
			case 1:		pool[h]->point = value; break;			    //1 �������� ������������ ���� ����
			case 4:		pool[h]->lot_step = value; break;			//4 ����������� ��� ���������� ����
			case 5:		pool[h]->lot_min = value; break;			//5 ����������� ���
			case 6:		pool[h]->lot_max = value; break;			//6 ������������ ���
			case 7:		pool[h]->min_sl_tp = value; break;			//7 ����������� ���������� �� ��������� ��� �����������
			case 8:		pool[h]->freeze = value; break;				//8 ���������� ��������� �������
			case 55:	pool[h]->_step = value; break;				//55 ������� ���, ����������� ��� (��� �������� �����)
			//case 56:	pool[h]->_step_mult = value; break;			//56 ��������� ����
			case 57:	pool[h]->_takeprofit = value; break;		//57 ������� ����������, ����������� (��� �������� �����)
			//case 58:	pool[h]->_tp_mult = value; break;			//58 ��������� �����������
			//case 60:	pool[h]->_tr_stop = value; break;			//60 �������� �������� �����
			//case 61:	pool[h]->_tr_stop_mult = value; break;		//61 ��������� �������� �����
			//case 62:	pool[h]->_tr_step = value; break;			//62 �������� ��������� ����
			//case 63:	pool[h]->_tr_step_mult = value; break;		//63 ��������� ��������� ����
			case 65:	pool[h]->_av_lot = value; break;			//65 ��� � �������� ���������� ����������� ������� ����������
			case 67:	pool[h]->_pips_mult = value; break;			//67 ��������� �������
			case 69:	pool[h]->_sell_lot = value; break;			//69 ��������� ��� �� �������
			case 97:	pool[h]->_buy_lot = value; break;			//97 ��������� ��� �� �������
			case 70:	pool[h]->_maxlot = value; break;			//70 ������������ ���
			case 71:	pool[h]->_lot_hadge_mult = value; break;	//71 ������� ������������
			case 72:	pool[h]->_regres_mult = value; break;		//72 ������� ���������
			case 74:	pool[h]->_trend_lot_mult = value; break;	//74
			case 75:	pool[h]->_trend_progress = value; break;	//75
			case 77:	pool[h]->_repeat_lot_mult = value; break;	//77
			case 78:	pool[h]->_repeat_progress = value; break;	//78
			//case 79:	pool[h]->_period = value; break;			//79
			case 80:	pool[h]->_deviation = value; break;			//80
			case 81:	pool[h]->_stoploss = value; break;			//81 ��������
			case 85:	pool[h]->_basket_hadge_mult = value; break; //85 ���� ��������� �������
			case 86:	pool[h]->_forward_step_mult = value; break; //86 ��������� ���� ��� ��������
			case 87:	pool[h]->_delta = value; break;		//87
			case 91:	pool[h]->_multf = value; break;		//91
			case 95:	pool[h]->_rollback = value; break;			//95
			case 96:	pool[h]->_weighthadge = value; break;		//96
			case 555:	pool[h]->PostInit(); break;
			default:	msg << "set double index not found: " << index << msg_box;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bp = true;
		msg << "c_setdouble ERROR: " << GetExceptionCode() << msg_box;
	};

}
//������������� ����� ����� ����������� MQL � DLL
_DLLAPI void __stdcall c_setvar(int h, int index, void* pointer)
{
	__try
	{
		switch(index)
		{
			case 102:	pool[h]->open_dd		= (double*)pointer; break;
			case 103:	pool[h]->total_lots		= (double*)pointer; break;
			case 104:	pool[h]->max_lvl		= (int*)pointer; break;
			case 105:	pool[h]->max_dd			= (double*)pointer; break;
			case 106:	pool[h]->indicator		= (double*)pointer; break;
			case 107:   pool[h]->count_p        = (int*)pointer; break;
			case 110:	pool[h]->o_ticket		= (int*)pointer; break;
			case 111:	pool[h]->o_type			= (int*)pointer; break;
			case 112:	pool[h]->o_lots			= (double*)pointer; break;
			case 113:	pool[h]->o_openprice	= (double*)pointer; break;
			case 114:	pool[h]->o_slprice		= (double*)pointer; break;
			case 115:	pool[h]->o_tpprice		= (double*)pointer; break;
			case 116:	pool[h]->indicator2		= (double*)pointer; break;
			case 200:	pool[h]->intret			= (int*)pointer; break;
			//case 117:	pool[h]->prev_indicator = (double*)pointer; break;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bp = true;
		msg << "c_setvar ERROR: " << GetExceptionCode() << msg_box;
	};

}
//������������� ����� ���������� �������, ��������� ���� ����������� ������ (������ �������� ���������� �� MQL), ������ ����������� ������������ � MQL ��������� �� ���������� �����
_DLLAPI void __stdcall c_refresh_init(int h, double ask, double bid, double equity)   
{
	pool[h]->refresh_init(ask, bid, equity);
}
//��������� ����� ����� � ����� ����� �������, � ������� ���������� ��� ���������
_DLLAPI int __stdcall c_refresh_order(int h, int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit=0)
{
	return(pool[h]->refresh_order(_ticket, _type, _lots, _openprice, _tp, _sl, _profit));
}
//������������ ���� ��� ������ ��������
_DLLAPI double __stdcall c_norm_lot(int h, double _lots)
{
	return(pool[h]->normlot(_lots));
}
_DLLAPI int __stdcall c_get_closed(int h)
{
	return(pool[h]->getclosed());
}
_DLLAPI void __stdcall c_refresh_prices(int h, double *_closes, double *_highs, double *_lows, int _bars)
{
	pool[h]->refresh_prices(_closes, _highs, _lows, _bars);
}

#define pi 3.14159265358979323846
_DLLAPI void __stdcall fcostransform(double a[], int tnn, int inversefct)
{
    int j;
    int n2;
    double sum;
    double y1;
    double y2;
    double theta;
    double wi;
    double wpi;
    double wr;
    double wpr;
    double wtemp;
    double twr;
    double twi;
    double twpr;
    double twpi;
    double twtemp;
    double ttheta;
    int i;
    int i1;
    int i2;
    int i3;
    int i4;
    double c1;
    double c2;
    double h1r;
    double h1i;
    double h2r;
    double h2i;
    double wrs;
    double wis;
    int nn;
    int ii;
    int jj;
    int n;
    int mmax;
    int m;
    int istep;
    int isign;
    double tempr;
    double tempi;

    if( tnn==1 )
    {
        y1 = a[0];
        y2 = a[1];
        a[0] = 0.5*(y1+y2);
        a[1] = 0.5*(y1-y2);
        if( inversefct )
        {
            a[0] += a[0];
            a[1] += a[1];
        }
        return;
    }
    wi = 0;
    wr = 1;
    theta = pi/tnn;
    wtemp = sin(theta*0.5);
    wpr = -2.0*wtemp*wtemp;
    wpi = sin(theta);
    sum = 0.5*(a[0]-a[tnn]);
    a[0] = 0.5*(a[0]+a[tnn]);
    n2 = tnn+2;
    for(j = 2; j <= tnn/2; j++)
    {
        wtemp = wr;
        wr = wtemp*wpr-wi*wpi+wtemp;
        wi = wi*wpr+wtemp*wpi+wi;
        y1 = 0.5*(a[j-1]+a[n2-j-1]);
        y2 = a[j-1]-a[n2-j-1];
        a[j-1] = y1-wi*y2;
        a[n2-j-1] = y1+wi*y2;
        sum = sum+wr*y2;
    }
    ttheta = 2.0*pi/tnn;
    c1 = 0.5;
    c2 = -0.5;
    isign = 1;
    n = tnn;
    nn = tnn/2;
    j = 1;
    for(ii = 1; ii <= nn; ii++)
    {
        i = 2*ii-1;
        if( j>i )
        {
            tempr = a[j-1];
            tempi = a[j];
            a[j-1] = a[i-1];
            a[j] = a[i];
            a[i-1] = tempr;
            a[i] = tempi;
        }
        m = n/2;
        while(m>=2&&j>m)
        {
            j = j-m;
            m = m/2;
        }
        j = j+m;
    }
    mmax = 2;
    while(n>mmax)
    {
        istep = 2*mmax;
        theta = 2.0*pi/(isign*mmax);
        wpr = -2.0*pow(sin(0.5*theta),2);
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;
        for(ii = 1; ii <= mmax/2; ii++)
        {
            m = 2*ii-1;
            for(jj = 0; jj <= (n-m)/istep; jj++)
            {
                i = m+jj*istep;
                j = i+mmax;
                tempr = wr*a[j-1]-wi*a[j];
                tempi = wr*a[j]+wi*a[j-1];
                a[j-1] = a[i-1]-tempr;
                a[j] = a[i]-tempi;
                a[i-1] = a[i-1]+tempr;
                a[i] = a[i]+tempi;
            }
            wtemp = wr;
            wr = wr*wpr-wi*wpi+wr;
            wi = wi*wpr+wtemp*wpi+wi;
        }
        mmax = istep;
    }
    twpr = -2.0*pow(sin(0.5*ttheta),2);
    twpi = sin(ttheta);
    twr = 1.0+twpr;
    twi = twpi;
    for(i = 2; i <= tnn/4+1; i++)
    {
        i1 = i+i-2;
        i2 = i1+1;
        i3 = tnn+1-i2;
        i4 = i3+1;
        wrs = twr;
        wis = twi;
        h1r = c1*(a[i1]+a[i3]);
        h1i = c1*(a[i2]-a[i4]);
        h2r = -c2*(a[i2]+a[i4]);
        h2i = c2*(a[i1]-a[i3]);
        a[i1] = h1r+wrs*h2r-wis*h2i;
        a[i2] = h1i+wrs*h2i+wis*h2r;
        a[i3] = h1r-wrs*h2r+wis*h2i;
        a[i4] = -h1i+wrs*h2i+wis*h2r;
        twtemp = twr;
        twr = twr*twpr-twi*twpi+twr;
        twi = twi*twpr+twtemp*twpi+twi;
    }
    h1r = a[0];
    a[0] = h1r+a[1];
    a[1] = h1r-a[1];
    a[tnn] = a[1];
    a[1] = sum;
    j = 4;
    while(j<=tnn)
    {
        sum = sum+a[j-1];
        a[j-1] = sum;
        j = j+2;
    }
    if( inversefct )
    {
        for(j = 0; j <= tnn; j++)
        {
            a[j] = a[j]*2/tnn;
        }
    }
    return;
}
#pragma endregion
