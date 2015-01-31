#pragma once

#include "Order.cpp"

namespace fxc {

	class Parameters
	{
		public:
			//��������� �����
			double   point;			    //1 �������� ������������ ���� ����
			int      digits;				//2 ���������� ���������� ������ ��� �����������
			char     symbol[10];			//3 �������� �������
			double   lot_step;			//4 ����������� ��� ���������� ����
			double   lot_min;				//5 ����������� ���
			double   lot_max;				//6 ������������ ���
			double   min_sl_tp;			//7 ����������� ���������� �� ��������� ��� �����������
			double   freeze;				//8 ���������� ��������� �������
			int      is_optimization;		//9 ���� �����������
			int      is_visual;			//10 ���� ����������� ������
			int      is_testing;			//11 ���� ������������
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
			int     _opp_close;			//97
			//����� ����������
			//double  ask;				//100
			//double  bid;				//101
			double*	 open_dd;			//102 ������� �������� �������� �� ������� ���� ��������
			double*	 total_lots;		//103 ��������� ����� ����� �� ������� ����
			int*	 max_lvl;			//104
			double*	 max_dd;			//105
			double*	 indicator;			//106
			int*	 count_p;			//107

			int*	 o_ticket;			//110
			int*	 o_type;			//111
			double*  o_lots;			//112
			double*	 o_openprice;		//113
			double*	 o_slprice;			//114
			double*	 o_tpprice;			//115
			double*	 indicator2;		//116
			double	 prev_indicator;
			int*     intret;            //200 ��� �������

			Order    cur_order;
			double   c_weight;
			int      c_index;
			bool     c_all;

			bool*    isRunAllowed;
	};

}