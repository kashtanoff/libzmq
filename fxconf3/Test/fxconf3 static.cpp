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

#define	debug	0

#pragma region ����� ��������� � �����������
#define	num_orders		100
#define OP_BUY			0
#define OP_SELL			1
#define OP_BUYLIMIT		2
#define OP_SELLLIMIT	3
#define OP_BUYSTOP		4
#define OP_SELLSTOP		5
#define Pool_Size		16

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
	double	_step_mult;			//56 ��������� ����
	double	_takeprofit;        //57 ������� ����������, ����������� (��� �������� �����)
	double	_tp_mult;			//58 ��������� �����������
	int		_forward_lvl;		//59 � ������ ������ ���������� ���������� ������
	double	_tr_stop;			//60 �������� �������� �����
	double	_tr_stop_mult;		//61 ��������� �������� �����
	double  _tr_step;			//62 �������� ��������� ����
	double  _tr_step_mult;		//63 ��������� ��������� ����
	int		_av_lvl;			//64 ������� ����������
	double	_av_lot;			//65 ��� � �������� ���������� ����������� ������� ����������
	int		_op_av_lvl;			//66 ������� ������ ���������������� ����������
	double	_pips_mult;			//67 ��������� �������
	int		_base_pips;			//68 ����� ������ ������� � �������� ���� � �� � ����������
	double	_minlot;			//69 ����������� ���
	double  _maxlot;			//70 ������������ ���
	double	_lot_hadge_mult;	//71 ������� ������������
	double	_regres_mult;		//72 ������� ���������
	int		_trend_lvl;			//73
	double	_trend_lot_mult;	//74
	double	_trend_progress;	//75
	int		_repeat_lvl;		//76
	double	_repeat_lot_mult;	//77
	double	_repeat_progress;	//78
	double	_rsi_delta;			//79
	double	_rsi_mult;			//80
	double	_stoploss;          //81 ��������
	int		_attemts;			//82
	int		_auto_mm;			//83
	int		_mm_equ;			//84
	double	_basket_hadge_mult; //85
	double	_forward_step_mult;	//86
	double	_rsi_delta2;		//87
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
	double	*prev_indicator;
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

	DILLER(PARAMETERS *_params, int _type)
	{
		params = _params;
		type = _type;
		tp_peak = 0;
		step_peak = maxmin[type];
		orders.clear();
		prev_lvl = 0;
		level = 0;
		sorted = false;
		last = first = nullptr;
	}
	void ResetTick()
	{
		prev_lvl	= level;
		prev_lots	= last? last->lots: 0;
		level		= 0;
		orders.clear();
		*(params->open_dd + type)		= 0.0;
		*(params->total_lots + type)	= 0.0;
		*(params->count_p + type)		= prev_lvl;
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
		double weight = order_weight(params->cur_order.openprice, mpc, params->cur_order.lots);
		if (params->c_weight + weight < 0.0)  //���� ������ ��������� ����� �����
		{
			//msg << "opp_partial close\r\n";
			//msg << "opp_�_weight=" << c_weight * 10000 << "\r\n";
			//msg << "opp_order weight=" << weight * 10000 << "\r\n";
			double min_weight = order_weight(params->cur_order.openprice, mpc, params->_minlot);
			//msg << "opp_min weight=" << min_weight * 10000 << "\r\n";
			if (params->c_weight + min_weight > 0.0)  //���� ����� ��������� ������ ����������� ���
			{
				*params->o_ticket = params->cur_order.ticket;
				*params->o_openprice = mpc;
				*params->o_lots = floor(params->c_weight / abs(min_weight)) * params->lot_step;
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
		*params->o_ticket = params->cur_order.ticket;
		*params->o_openprice = mpc;
		*params->o_lots = params->cur_order.lots;
		//msg << "opp_end weight2=" << c_weight * 10000 << "\r\n";
		//msg << "opp_close_one" << "\r\n";
		params->c_index++;
		
		return(true);
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
	double  steps[50];
	double	tps[50];
	double	profits[50];
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


		
		k=0;
		m_index = 0;
		c_index = -1;
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
			steps[i] = norm(max(0.0010, _step * pow(_step_mult, i)));
			tps[i] = norm(max(0.0010, _takeprofit * pow(_tp_mult, i)));
			profits[i] = _takeprofit * pow(_pips_mult, i);
			//msg << "steps[" << i << "]=" << steps[i] << ", tps = " << tps[i] << "\r\n";
		}
		*indicator = 10;
		*prev_indicator = -50;
		*indicator2 = -50;
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
	}


	void calc_first_lot()
	{
#if debug
		ShowInfo("calc_first_lot", "start");
#endif
		curdil->cur_av_lvl = _av_lvl;		//��������������� ������� ������� ���������� �� ��������
		if (_auto_mm > 0)				//���� ������� ��������������
			base_lot = normlot(floor(equity/_mm_equ) * _minlot);
		else
			base_lot = _minlot;
		*o_lots = base_lot;
		//msg << "base_lot=" << base_lot << "\r\n";
#if debug
		ShowInfo("opposite", curdil->opposite->level);
		ShowInfo("trend lvl", _trend_lvl);
#endif
		if (curdil->opposite->level >= _trend_lvl)	//������������ ���������� ��� ������
			{*o_lots = max(*o_lots, base_lot * _trend_lot_mult * pow(_trend_progress, curdil->opposite->level - _trend_lvl));
#if debug
		ShowInfo("trend", *o_lots);
#endif
		}
		else if (curdil->prev_lvl >= _repeat_lvl)		//������������ ���������� ��� �������
			*o_lots = max(*o_lots, base_lot * _repeat_lot_mult * pow(_repeat_progress, curdil->prev_lvl - _repeat_lvl));
		//msg << "repeat: " << *o_lots << "\r\n";}
		if ((curdil->type == OP_BUY && *indicator < (_rsi_delta - 100)) ||
			(curdil->type == OP_SELL && *indicator > -_rsi_delta))	//������������ ���������� �� ����������
			{*o_lots = max(*o_lots, base_lot * _rsi_mult);
#if debug
		ShowInfo("indicator", *o_lots);
#endif
		}
		if (curdil->opposite->last)
			*o_lots = max(*o_lots, curdil->opposite->last->lots * _lot_hadge_mult);		//������������ ������� ������������
		*o_lots = max(*o_lots, *(total_lots + curdil->opposite->type) * _basket_hadge_mult);
		//msg << "hadge(" << _hadge_mult << "): " << *o_lots << "\r\n";
		*o_lots = max(*o_lots, curdil->prev_lots * _regres_mult);		//���������
		//msg << "regres: " << *o_lots << msg_box;
		*o_lots = min(*o_lots, _maxlot);
		if (_base_pips)
			base_lot = *o_lots;
	}
	bool calc_first()
	{
		ShowInfo("calc_first", "start");
		/*if (_tr_step > 0 && curdil->delta(curdil->step_peak, curdil->mpo) < _tr_step)
		{
			curdil->step_peak = curdil->best_price(curdil->step_peak, curdil->mpo);
			ShowInfo("calc_first trailing peak", curdil->step_peak);
			ShowInfo("calc_first delta", curdil->delta(curdil->step_peak, curdil->mpo));
			return(false);
		}*/
		*o_openprice = curdil->mpo;
		*o_tpprice = _tr_stop? curdil->tp(*o_openprice, _takeprofit * 10) : curdil->tp(*o_openprice, _takeprofit);
		ShowInfo("calc_first: _takeprofit", _takeprofit);
		ShowInfo("calc_first: tp_price",  curdil->tp(*o_openprice, _takeprofit));
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
#if debug 
		ShowInfo("calc_forward", "start");
#endif
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
		*o_tpprice	= _tr_stop? curdil->tp(*o_openprice, _takeprofit * 10): curdil->tp(*o_openprice, _takeprofit);
		*o_slprice	= curdil->sl(*o_openprice, _stoploss); 
		*intret = check_new();
		if (*intret > 0)
		{
#if debug 
			ShowInfo("*** calc_forward check error", *intret);
#endif
			return(false);}
		return(true);
	}
	bool calc_next()
	{
#if debug
		ShowInfo("calc_next", "start");
#endif
		double nextStep = steps[curdil->level-1];//_step * pow(_step_mult, curdil->level);
		*o_openprice  = curdil->sl(curdil->last->openprice, nextStep);
		double d = curdil->delta(*o_openprice, curdil->mpo);  //���������� �� �������� ������, ���� �������������, �� ��������, ���� ������� �� �����
		if (_tr_step) //------------ �������� ���� --------------
		{
#if debug 
			ShowInfo("calc_next tr_step, d", d);
#endif
			//double trStep = _tr_step * pow(_tr_step_mult, curdil->opposite->level);
			if (d<=0)//d <= 0 && curdil->delta(curdil->step_peak, curdil->mpo) >= _tr_step)
			{
				*o_openprice = curdil->mpo;
				*o_type = curdil->type;
				//msg << "trailng step!" << msg_box;
			}
			else
			{
#if debug 
				ShowInfo("calc_next tr cant open, delta", curdil->delta(curdil->step_peak, curdil->mpo));
#endif
				curdil->step_peak = curdil->best_price(curdil->step_peak, curdil->mpo);
				return(false);
			}
		}
		else 
		{
			ShowInfo("calc_next fixed step, d", d);
			if (d > min_sl_tp)  //���� ����� ��������� �������
				*o_type = curdil->type + 2;
			else if (d <= 0)  //���������� �� �����
			{
				*o_openprice = curdil->mpo;
				*o_type = curdil->type;
			}
			else   //���� �� ����� ��������� �� ������� �� �� �����
			{
				ShowInfo("calc_next fixed step, min_sl_tp", min_sl_tp);
				return(false);}
		}
		*o_slprice = curdil->sl(*o_openprice, _stoploss);
		if (curdil->last->lots >= _av_lot)
		{
			if (curdil->cur_av_lvl == _av_lvl && curdil->level < _av_lvl)
				curdil->cur_av_lvl = max(1, curdil->level);
			else
				curdil->cur_av_lvl = max(1, curdil->cur_av_lvl-1);
		}
		double nextTP = tps[curdil->level-1];
		*o_tpprice = curdil->tp(*o_openprice, nextTP);
		ShowInfo("calc_next nextTP", nextTP);
		if (_tr_stop)
		{
			//*o_lots = curdil->last->lots * _pips_mult;
			//*o_lots = max(*o_lots, (_takeprofit * _minlot - curdil->order_weight(curdil->first->openprice, *o_tpprice, _minlot)) / nextTP);  //��� ������ ������� ����������� ����� ���� ������� ������
			double nextProfit = profits[curdil->level-1];
			*o_lots = (nextProfit * base_lot - curdil->basket_weight(*o_tpprice, curdil->cur_av_lvl)) / nextTP;
			*o_tpprice = curdil->tp(*o_openprice, nextTP * 10);
		}
		else
		{
			double nextProfit = profits[curdil->level-1];
			ShowInfo("calc_next level", curdil->level);
			ShowInfo("calc_next nextProfit", nextProfit);
			ShowInfo("calc_next base_lot", base_lot);
			*o_lots = (nextProfit * base_lot - curdil->basket_weight(*o_tpprice, curdil->cur_av_lvl)) / nextTP;
		}
		*o_lots = min(*o_lots, _maxlot);
		*intret = check_new();
		if (*intret > 0)
		{
			ShowInfo("*** calc_next check error", *intret);
			return(false);}
		/*msg << "calc_next()\r\n";
		msg << "level=" << level << "\r\n";
		msg << "nextStep=" << nextStep << "\r\n";
		msg << "nextTP=" << nextTP << "\r\n";
		msg << "nextProfit=" << nextProfit << "\r\n";
		msg << "openprice=" << *o_openprice << "\r\n";
		msg << "lots=" << *o_lots << "\r\n";
		msg << "delta: " << d << msg_box;
		*/
		*intret = curdil->level + 1;
		ShowInfo("calc_next success", curdil->type);
		return(true);
	}
	bool calc_oposite()
	{
		ShowInfo("calc_oposite", "start");
		if (!_tr_stop && curdil->opposite->level > _op_av_lvl && curdil->opposite->GetOrder())
		{
			//msg << "try calc oposite" << msg_box;
			double tp_price = curdil->tp(curdil->mpo, _takeprofit);
			double oplots = (_takeprofit * base_lot - curdil->opposite->order_weight(cur_order.openprice, tp_price, cur_order.lots))/_takeprofit;
			if (oplots > _maxlot)  //���� ��� ��������� ��������, �� ������� �� ���������
				return(false);
			*o_lots = max(*o_lots, oplots);
			*o_ticket = cur_order.ticket;
			*o_openprice = cur_order.openprice;
			*o_slprice = tp_price;
			*o_tpprice = cur_order.tpprice;
			*intret = check_mod();
			if (*intret > 0)
				return(false);
			return(true);
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
	bool trailing_stop()
	{
		if (!_tr_stop || !curdil->level)
			return(false);
		ShowInfo("trailing_stop", "started");
		double d = curdil->delta(curdil->last->openprice, curdil->mpc);
		double nextTP = tps[curdil->level-1];//_takeprofit * pow(_tp_mult, curdil->level-1);
		//double nextTR = _tr_stop * pow(_tr_stop_mult, curdil->level-1);
		if (!curdil->tp_peak)
		{
			if (d > nextTP)
				curdil->tp_peak = curdil->mpc;
			return(false);
		}
		else
		{
			curdil->tp_peak = curdil->opposite->best_price(curdil->tp_peak, curdil->mpc);
			if (d <= (nextTP + 0.0001)  || curdil->delta(curdil->mpc, curdil->tp_peak) >= _tr_stop)//nextTR)
			{
				curdil->tp_peak = 0;
				return(true);
			}
			return(false);
		}
	}
	bool close_profit()
	{
		//msg << "close profit, type: " << type << "\r\n";
		//msg << "index=" << c_index << "\r\n";
		if (c_index < 0)  //�������������
		{
			c_index = 0;
			c_weight = curdil->basket_weight(curdil->mpc);
			if (c_weight > 0.0)   //���� ��� ���� ����� �������������, �� ��������� ��� �����
				c_all = true;
			else
			{
				c_weight = curdil->order_weight(curdil->last->openprice, curdil->mpc, curdil->last->lots - _minlot);
				c_all = false;
			}
		}
		if (c_all && c_weight>0 && curdil->opposite->level >= _op_av_lvl)   //�������� ����������
		{
			//msg << "opposite average: " << count[1-type] << "\r\n";
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
			double min_weight = curdil->order_weight(cur_order.openprice, curdil->mpc, _minlot);
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
	bool signal()
	{
		ShowInfo("signal prev", *prev_indicator);
		ShowInfo("signal cur", *indicator);
		ShowInfo("rsi delta", _rsi_delta);
		if (curdil->type)
		{
			
			if (*indicator >= -_rsi_delta)
				*prev_indicator = max(*prev_indicator, *indicator);
			if (*prev_indicator >= - _rsi_delta && *indicator2 < -_rsi_delta2)
			{
				*prev_indicator = -50;
				//bp = true;
				return true;}
		}
		else
			if (*indicator <= (_rsi_delta - 100))
				*prev_indicator = min(*prev_indicator, *indicator);
			if (*prev_indicator <= (_rsi_delta - 100) && *indicator2 > (_rsi_delta2 - 100))
			{
				*prev_indicator = -50;
				//bp = true;
				return true;}
		return false;
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
		//__try
		//{
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
			*indicator = 10;
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
			//msg << "refresh init done" << msg_box;
		/*}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			msg << "refresh_init ERROR: " << GetExceptionCode() << msg_box;
		};*/
	}
	//3.1 ��������� ����� ����� � ����� ����� �������, � ������� ���������� ��� ���������
	int refresh_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit=0)
	{
		//__try
		//{
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
		/*}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			msg << "refresh_order ERROR: " << GetExceptionCode() << msg_box;
		};*/

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
		//__try
		//{
		//msg << "*" << msg_box;
		ShowInfo("======== get_job", "start ========");
		while(true)		//����� ������ ��� �� �������� � MQL
		{
			/*msg << "Simbiot getjob\r\n";
			msg << "type = " << type << "\r\n";
			msg << "k = " << k << "\r\n";
			msg << "indicator = " << *indicator << msg_box;*/
			ShowInfo("----- getjob work", k);
			switch(k)
			{
			case 0: //����� ������ ����
				Sort();
				if (!curdil->level)		//���� ��� ������� � �����
				{
					if (!_stop_new[curdil->type])	//���� �� ��������� �������� ����� �����
						{k=1; 
						if (*indicator==10)
							return(5); //����� ������ ����������
						break;
						}	
					k=2; break;
				}
				else if (!_stop_avr[curdil->type] && curdil->level < _max_grid_lvl)   //���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
				{
					//bp = true;
					if (_tr_step && curdil->level < curdil->prev_lvl)  //���� ����� ���������, �� ���������� ��� �������� �����
						curdil->step_peak = curdil->mpo;
					if (_tr_stop && curdil->level > curdil->prev_lvl)  //���� ����� ���������, �� ���������� ��� �������� �����
						curdil->tp_peak = 0;
					if (curdil->opposite->level >= _forward_lvl && !curdil->ord_stop)   //���� ��������� �������� � ��� ����� ���������
						{k=5; 
						if (*indicator==10)
							return(5); //����� ������ ����������
						break;
						}	
					k=6; 
					if (*indicator==10)
						return(5); //����� ������ ����������
					break;   //����������� �������� �����
				}
				else  //���� ���� ������, �� �� ��������� ��������� -> ������� �������
					k=3;
				break;
			case 1: //�������� ������� ������: �������� ������ ����������
				if (!signal())
				{
					k = 2;
					break;
				}
				bp = true;
				calc_first_lot();
				k=8;
				if (calc_oposite())		//������������ �������� ����������
				{
					ShowInfo("modorder opposite", "");
					return(2);}
				break;
			case 2: //�������� ���������������� ���� ������
				k=3;
				if (curdil->opposite->ord_stop)
				{
					*o_ticket = curdil->opposite->ord_stop->ticket;
					ShowInfo("delorder stop opposite", *o_ticket);
					return(3);  //������� ���� �����
				}
				break;
			case 3:	//�������� ������ ���� ������
				k=4;
				if (curdil->ord_stop)
				{
					*o_ticket = curdil->ord_stop->ticket;
					ShowInfo("delorder stop", *o_ticket);
					return(3);	//������� ���� �����
				}
				break;
			case 4: //�������� ��������� ������
				k=9;
				if (curdil->ord_limit)
				{
					*o_ticket = curdil->ord_limit->ticket;
					ShowInfo("delorder limit", *o_ticket);
					return(3);	//������� ������� �����
				}
				break;
			case 5:	//�������� ��������
				k=6;
				if (calc_forward())
				{//msg << "open_forward" << msg_box;
					ShowInfo("neworder forward", *o_lots);
				return(1);}
				break;
			case 6:
				if (!curdil->ord_limit)	//���� ��� ����������
					{k=7; break;}  //������� �����������
				else if (curdil->prev_lvl > curdil->level)   //���� ��������� ����� ����� � ���� �������
					{k = 4; break;}		//������� ���������
				k = 9; break;
			case 7:  //������� �����������, ��������� ����������� ������
				if (!_tr_stop && move_tp())	//���� ���� ��� �������, �������
					return(2);
				k=9;
				if (!signal())
					break;
				if (calc_next()) //���� ����� ��������� ������� ��� �� �����, ����������
				{
					//bp = true;
					ShowInfo("neworder next", *o_lots);
					return(1);}
				break;
			case 8:  //��������� ������ �����
				k=2;
				if (calc_first())
				{
					ShowInfo("neworder first", *o_lots);
					return(1);}		//������� ������ �����
				break;
			case 9:  //��������� ������������ ������������ (�������������) �����������
				if (trailing_stop())
					{k = 10; break; }
				k = 100; 
				break;
			case 10:  //������� ��� ��� ����� � ������
				if (close_profit())
				{
					ShowInfo("closeorder profit", *o_lots);
					return(4);}
				k = 100;
				break;
			case 100: //���������� ������, ��������� ������ ��� ��������� ���������
				k = 0;  //��������� ������ ������
				//ShowInfo("k100 start", "");
				if (curdil->type == 0)
					{curdil = dillers[1];
					ShowInfo("======== Start Sell ========", "");
					break;}	//��� ������� ������ ��� �������
				curdil = dillers[0];
				ShowInfo("======== get_job", "end ========", true);
				bp = false;
				return(0);	//������������ �������� � ��������� ��������
			}
		
		}
		/*} __except(EXCEPTION_EXECUTE_HANDLER)
		{
			ShowInfo("get_job ERROR", GetExceptionCode(), true);
		};*/
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
	if (first_run)
	{
		memset(pool, 0, Pool_Size);
		first_run = false;
	}


	for (int h=0; h<Pool_Size; h++)
	{
		if (pool[h] == nullptr)
		{
			//msg << "Pool position: " << h << "\r\n";
			pool[h] = new SIMBIOT();
			return(h);
		}
	}
	msg << "Pool position not found" << msg_box;
	return(-1);
}
//����������� ������ � ������ � ����
_DLLAPI void __stdcall c_deinit(int h)   
{
	delete pool[h];
	pool[h] = nullptr;
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
			case 68:	pool[h]->_base_pips = value;			//68 ����� ������ ������� � �������� ���� � �� � ����������
			case 73:	pool[h]->_trend_lvl = value;			//73
			case 76:	pool[h]->_repeat_lvl = value;			//76
			case 82:	pool[h]->_attemts = value;				//82
			case 83:	pool[h]->_auto_mm = value;				//83
			case 84:	pool[h]->_mm_equ = value;				//84
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
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
			case 56:	pool[h]->_step_mult = value; break;			//56 ��������� ����
			case 57:	pool[h]->_takeprofit = value; break;		//57 ������� ����������, ����������� (��� �������� �����)
			case 58:	pool[h]->_tp_mult = value; break;			//58 ��������� �����������
			case 60:	pool[h]->_tr_stop = value; break;			//60 �������� �������� �����
			case 61:	pool[h]->_tr_stop_mult = value; break;		//61 ��������� �������� �����
			case 62:	pool[h]->_tr_step = value; break;			//62 �������� ��������� ����
			case 63:	pool[h]->_tr_step_mult = value; break;		//63 ��������� ��������� ����
			case 65:	pool[h]->_av_lot = value; break;			//65 ��� � �������� ���������� ����������� ������� ����������
			case 67:	pool[h]->_pips_mult = value; break;			//67 ��������� �������
			case 69:	pool[h]->_minlot = value; break;			//69 ����������� ���
			case 70:	pool[h]->_maxlot = value; break;			//70 ������������ ���
			case 71:	pool[h]->_lot_hadge_mult = value; break;	//71 ������� ������������
			case 72:	pool[h]->_regres_mult = value; break;		//72 ������� ���������
			case 74:	pool[h]->_trend_lot_mult = value; break;	//74
			case 75:	pool[h]->_trend_progress = value; break;	//75
			case 77:	pool[h]->_repeat_lot_mult = value; break;	//77
			case 78:	pool[h]->_repeat_progress = value; break;	//78
			case 79:	pool[h]->_rsi_delta = value; break;			//79
			case 80:	pool[h]->_rsi_mult = value; break;			//80
			case 81:	pool[h]->_stoploss = value; break;			//81 ��������
			case 85:	pool[h]->_basket_hadge_mult = value; break; //85 ���� ��������� �������
			case 86:	pool[h]->_forward_step_mult = value; break; //86 ��������� ���� ��� ��������
			case 87:	pool[h]->_rsi_delta2 = value; break;		//87
			case 555:	pool[h]->PostInit(); break;
			default:	msg << "set double index not found: " << index << msg_box;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
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
			case 117:	pool[h]->prev_indicator = (double*)pointer; break;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
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
	return(pool[h]->refresh_order(_ticket, _type, _lots, _openprice, _tp, _sl));
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
#pragma endregion
