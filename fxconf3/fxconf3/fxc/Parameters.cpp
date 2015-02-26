#pragma once

#include "vector"

#include "../Debug.h"
#include "../MqlUtils.cpp"
#include "Order.cpp"

namespace fxc {

	#pragma pack(1)
	struct TradeAction {
		double o_lots;      // 8 bytes
		double o_openprice; // 8 bytes
		double o_tpprice;   // 8 bytes
		double o_slprice;   // 8 bytes
		int    o_ticket;    // 4 bytes
		int    o_type;      // 4 bytes
		int    intret;      // 4 bytes
		int    actionId;    // 4 bytes
		MqlString comment;  // 12 bytes
	};

	class Parameters
	{
		public:
			//Константы рынка
			double   input_point;			    //1 значение минимального шага цены
			int      input_digits;				//2 количество десятичных знаков для инструмента #unused
			char     symbol[10];			//3 название символа
			double   input_lot_step;			//4 минимальный шаг приращения лота #unused
			double   input_lot_min;				//5 минимальный лот
			double   input_lot_max;				//6 максимальный лот
			double   input_min_sl_tp;			//7 минимальное расстояние до стоплосса или тейкпрофита
			double   input_freeze;				//8 расстояние заморозки ордеров
			int      input_is_optimization;		//9 флаг оптимизации
			int      input_is_visual;			//10 флаг визуального режима
			int      input_is_testing;			//11 флаг тестирования #unused
			//Параметры советника
			int		input_stop_new[2];		//50, 51 Остановить открытие новой сетки
			int		input_stop_avr[2];		//52, 53 Остановить открытие новой ступени
			int		input_max_grid_lvl;		//54 Максимальный уровень сетки
			double	input_step;				//55 базовый шаг, минимальный шаг (для трейлинг степа)
			//double	_step_mult;			//56 множитель шага
			double	input_takeprofit;        //57 базовый тейкпрофит, минимальный (для трейлинг стопа)
			//double	_tp_mult;			//58 множитель тейкпрофита
			int		input_forward_lvl;		//59 с какого уровня выставлять форвардные сделки
			//double	_tr_stop;			//60 величина трейлинг стопа
			//double	_tr_stop_mult;		//61 множитель трейлинг стопа
			//double  _tr_step;			//62 величина трейлинга шага
			//double  _tr_step_mult;		//63 множитель трейлинга шага
			int		input_av_lvl;			//64 ступень усреднения
			double	input_av_lot;			//65 лот с которого начинается уменьшаться ступень усреднения
			int		input_op_av_lvl;			//66 уровень начала противоположного усреднения
			double	input_pips_mult;			//67 множитель прибыли
			int		input_safe_copy;			//68 вести расчет прибыли с базового лота а не с начального
			double	input_sell_lot;			//69 начальный лот на продажу
			double	input_buy_lot;			//97 начальный лот на покупку
			double  input_maxlot;			//70 максимальный лот
			double	input_lot_hadge_mult;	//71 процент хэджирования
			double	input_regres_mult;		//72 процент затухания
			int		input_trend_lvl;			//73
			double	input_trend_lot_mult;	//74
			double	input_trend_progress;	//75
			int		input_repeat_lvl;		//76
			double	input_repeat_lot_mult;	//77
			double	input_repeat_progress;	//78
			int		input_period;			//79
			double	input_deviation;			//80
			double	input_stoploss;          //81 стоплосс
			int		input_attemts;			//82 #unused
			int		input_auto_mm;			//83
			int		input_mm_equ;			//84
			double	input_basket_hadge_mult; //85
			double	input_forward_step_mult;	//86
			double	input_delta;				//87
			int		input_first_free;		//88
			int		input_new_bar;			//89
			int		input_free_lvl;			//90
			double  input_multf;				//91
			int		input_periodf2;			//92
			int		input_periodf3;			//93
			int		input_buf_len;			//94
			double	input_rollback;			//95
			double	input_weighthadge;		//96
			int     input_opp_close;			//97
			//Общие переменные
			//double  ask;				//100
			//double  bid;				//101
			double*	 ext_open_dd;			//102 подсчет открытой просадки по каждому типу отдельно
			double*	 ext_total_lots;		//103 суммарный объем лотов по каждому типу
			int*	 ext_max_lvl;			//104 #unused
			double*	 ext_max_dd;			//105 #unused
			double*	 ext_indicator;			//106
			int*	 ext_count_p;			//107

			double*	 ext_indicator2;		//116 #unused
			double	 prev_indicator;

			Order    cur_order;
			double   c_weight;
			int      c_index;
			bool     c_all;

			bool*                     ext_isRunAllowed;
			std::vector<TradeAction*> ext_tradeActions;
	};

}