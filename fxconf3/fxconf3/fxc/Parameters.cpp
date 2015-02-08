#pragma once

#include "Order.cpp"

namespace fxc {

	class Parameters
	{
		public:
			//Константы рынка
			double   point;			    //1 значение минимального шага цены
			int      digits;				//2 количество десятичных знаков для инструмента
			char     symbol[10];			//3 название символа
			double   lot_step;			//4 минимальный шаг приращения лота
			double   lot_min;				//5 минимальный лот
			double   lot_max;				//6 максимальный лот
			double   min_sl_tp;			//7 минимальное расстояние до стоплосса или тейкпрофита
			double   freeze;				//8 расстояние заморозки ордеров
			int      is_optimization;		//9 флаг оптимизации
			int      is_visual;			//10 флаг визуального режима
			int      is_testing;			//11 флаг тестирования
			//Параметры советника
			int		_stop_new[2];		//50, 51 Остановить открытие новой сетки
			int		_stop_avr[2];		//52, 53 Остановить открытие новой ступени
			int		_max_grid_lvl;		//54 Максимальный уровень сетки
			double	_step;				//55 базовый шаг, минимальный шаг (для трейлинг степа)
			//double	_step_mult;			//56 множитель шага
			double	_takeprofit;        //57 базовый тейкпрофит, минимальный (для трейлинг стопа)
			//double	_tp_mult;			//58 множитель тейкпрофита
			int		_forward_lvl;		//59 с какого уровня выставлять форвардные сделки
			//double	_tr_stop;			//60 величина трейлинг стопа
			//double	_tr_stop_mult;		//61 множитель трейлинг стопа
			//double  _tr_step;			//62 величина трейлинга шага
			//double  _tr_step_mult;		//63 множитель трейлинга шага
			int		_av_lvl;			//64 ступень усреднения
			double	_av_lot;			//65 лот с которого начинается уменьшаться ступень усреднения
			int		_op_av_lvl;			//66 уровень начала противоположного усреднения
			double	_pips_mult;			//67 множитель прибыли
			int		_safe_copy;			//68 вести расчет прибыли с базового лота а не с начального
			double	_sell_lot;			//69 начальный лот на продажу
			double	_buy_lot;			//97 начальный лот на покупку
			double  _maxlot;			//70 максимальный лот
			double	_lot_hadge_mult;	//71 процент хэджирования
			double	_regres_mult;		//72 процент затухания
			int		_trend_lvl;			//73
			double	_trend_lot_mult;	//74
			double	_trend_progress;	//75
			int		_repeat_lvl;		//76
			double	_repeat_lot_mult;	//77
			double	_repeat_progress;	//78
			int		_period;			//79
			double	_deviation;			//80
			double	_stoploss;          //81 стоплосс
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
			//Общие переменные
			//double  ask;				//100
			//double  bid;				//101
			double*	 open_dd;			//102 подсчет открытой просадки по каждому типу отдельно
			double*	 total_lots;		//103 суммарный объем лотов по каждому типу
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
			int*     intret;            //200 для отладки

			Order    cur_order;
			double   c_weight;
			int      c_index;
			bool     c_all;

			bool*    isRunAllowed;
	};

}