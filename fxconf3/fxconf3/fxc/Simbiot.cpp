#pragma once

#include <unordered_map>

#include "../stdafx.h"
#include "../Defines.h"
#include "../Property.h"

#include "Parameters.cpp"
#include "OrdersManager.cpp"
#include "TradeManager.cpp"
#include "Diller.cpp"

namespace fxc {
	
	class Simbiot :
		public Parameters,
		public CPropertyList,
		public OrdersManager,
		public TradeManager
	{

		public:

			Simbiot(char* _symbol) {
				strcpy(symbol, _symbol);
				registerProps();
			}

			void postInit() {
				((OrdersManager*) this)->init(this);
				printRegisteredProps();
			}

			inline double normLot(double value) {
				value = int(ceil(value / input_lot_step)) * input_lot_step;
				value = max(value, input_lot_min);
				return min(value, input_lot_max);
			}

			inline double normPrice(double value) {
				return floor(value / input_point + 0.5) * input_point;
			}

		protected:

#pragma region Checks

			//Проверка уровня стоплосса или тейкпрофита
			inline bool check_sl(int type, double low_price, double high_price) {
				return !(low_price && high_price && dillers[type % 2]->delta(low_price, high_price) < input_min_sl_tp);
			}
			
			//Проверка на правильность параметров при открытии ордера
			inline int check_new(int type, double* lots, double* openprice, double* slprice, double* tpprice) {
				type = type % 2;
				if (
					type < 2 && // Для немедленного исполнения
					(
						!check_sl(type, *slprice, dillers[type]->mpc) ||
						!check_sl(type, dillers[type]->mpc, *tpprice)
					)
				) {
					return 201;
				}
				
				if (
					type > 1 &&  // Для отложенных ордеров
					(
						!check_sl(type, *openprice, *tpprice) ||
						!check_sl(type, *slprice, *openprice)
					)
				) {
					return 201;
				}
				
				if (
					(type == OP_BUYLIMIT || type == OP_SELLLIMIT) &&
					!check_sl(type, *openprice, dillers[type]->mpo)
				) {
					return 202;
				}
				
				if (
					(type == OP_BUYSTOP || type == OP_SELLSTOP) &&
					!check_sl(type, dillers[type]->mpo, *openprice)
				) {
					return 202;
				}

				*lots      = normLot(*lots);
				*openprice = normPrice(*openprice);
				*tpprice   = normPrice(*tpprice);
				*slprice   = normPrice(*slprice);

				return 0;
			}

#pragma endregion

		private:

			void registerProps() {
				Register("point",              &input_point);              //1 значение минимального шага цены
				Register("lot_step",           &input_lot_step);           //4 минимальный шаг приращения лота
				Register("lot_min",            &input_lot_min);            //5 минимальный лот
				Register("lot_max",            &input_lot_max);            //6 максимальный лот
				Register("min_sl_tp",          &input_min_sl_tp);          //7 минимальное расстояние до стоплосса или тейкпрофита
				Register("freeze",             &input_freeze);             //8 расстояние заморозки ордеров
				Register("_step",              &input_step);              //55 базовый шаг, минимальный шаг (для трейлинг степа)
				Register("_takeprofit",        &input_takeprofit);        //57 базовый тейкпрофит, минимальный (для трейлинг стопа)
				Register("_av_lot",            &input_av_lot);            //65 лот с которого начинается уменьшаться ступень усреднения
				Register("_pips_mult",         &input_pips_mult);         //67 множитель прибыли
				Register("_sell_lot",          &input_sell_lot);          //69 начальный лот на продажу
				Register("_buy_lot",           &input_buy_lot);           //97 начальный лот на покупку
				Register("_maxlot",            &input_maxlot);            //70 максимальный лот
				Register("_lot_hadge_mult",    &input_lot_hadge_mult);    //71 процент хэджирования
				Register("_regres_mult",       &input_regres_mult);       //72 процент затухания
				Register("_trend_lot_mult",    &input_trend_lot_mult);    //74
				Register("_trend_progress",    &input_trend_progress);    //75
				Register("_repeat_lot_mult",   &input_repeat_lot_mult);   //77
				Register("_repeat_progress",   &input_repeat_progress);   //78
				Register("_deviation",         &input_deviation);         //80
				Register("_stoploss",          &input_stoploss);          //81 стоплосс
				Register("_basket_hadge_mult", &input_basket_hadge_mult); //85 хэдж множитель корзины
				Register("_forward_step_mult", &input_forward_step_mult); //86 множитель шага при форварде
				Register("_delta",             &input_delta);             //87
				Register("_multf",             &input_multf);             //91
				Register("_rollback",          &input_rollback);          //95
				Register("_weighthadge",       &input_weighthadge);       //96

				Register("open_dd",     &ext_open_dd);     // 102
				Register("total_lots",  &ext_total_lots);  // 103
				Register("count_p",     &ext_count_p);     // 107

				//Register("indicator",  &ext_indicator);   // 106
				//Register("indicator2", &ext_indicator2);  // 116

				Register("digits",          &input_digits);          //2 количество десятичных знаков для инструмента
				Register("is_optimization", &input_is_optimization); //9 флаг оптимизации
				Register("is_visual",       &input_is_visual);       //10 флаг визуального режима
				Register("is_testing",      &input_is_testing);      //11 флаг тестирования
				Register("_stop_new[0]",    &input_stop_new[0]);     //50, 51 Остановить открытие новой сетки
				Register("_stop_new[1]",    &input_stop_new[1]);     //50, 51 Остановить открытие новой сетки
				Register("_stop_avr[0]",    &input_stop_avr[0]);     //52, 53 Остановить открытие новой ступени
				Register("_stop_avr[1]",    &input_stop_avr[1]);     //52, 53 Остановить открытие новой ступени
				Register("_max_grid_lvl",   &input_max_grid_lvl);    //54 Максимальный уровень сетки
				Register("_forward_lvl",    &input_forward_lvl);     //59 с какого уровня выставлять форвардные сделки
				Register("_av_lvl",         &input_av_lvl);          //64 ступень усреднения
				Register("_op_av_lvl",      &input_op_av_lvl);       //66 уровень начала противоположного усреднения
				Register("_safe_copy",      &input_safe_copy);       //68 вести расчет прибыли с базового лота а не с начального
				Register("_trend_lvl",      &input_trend_lvl);       //73
				Register("_repeat_lvl",     &input_repeat_lvl);      //76
				Register("_period",         &input_period);          //79
				Register("_attemts",        &input_attemts);         //82
				Register("_auto_mm",        &input_auto_mm);         //83
				Register("_mm_equ",         &input_mm_equ);          //84
				Register("_first_free",     &input_first_free);      //88
				Register("_new_bar",        &input_new_bar);         //89
				Register("_free_lvl",       &input_free_lvl);        //90
				Register("_periodf2",       &input_periodf2);        //92
				Register("_periodf3",       &input_periodf3);        //93
				Register("_buf_len",        &input_buf_len);         //94
				Register("_opp_close",      &input_opp_close);       //97 OppositeCLose;

				Register("isRunAllowed",    &ext_isRunAllowed);
			}

			void printRegisteredProps() {
				for (auto pair : PropertyList)
				{
					switch (pair.second.Type)
					{
						case PropBool : 
							msg << pair.first << ": " << *(pair.second.Bool) << "\r\n" << msg_box;
							break;
						case PropInt : 
							msg << pair.first << ": " << *(pair.second.Int) << "\r\n" << msg_box;
							break;
						case PropDouble : 
							msg << pair.first << ": " << *(pair.second.Double) << "\r\n" << msg_box;
							break;
						case PropBoolPtr :
							if (pair.second.BoolPtr == nullptr) {
								msg << pair.first << ": " << "n/a [0x" << pair.second.BoolPtr << "]\r\n" << msg_box;
							} else {
								msg << pair.first << ": "
									<< **(pair.second.BoolPtr)
									<< " [0x" << *(pair.second.BoolPtr) << "]"
									<< "\r\n" << msg_box;
							}
							break;
						case PropIntPtr :
							if (pair.second.IntPtr == nullptr) {
								msg << pair.first << ": " << "n/a [0x" << pair.second.IntPtr << "]\r\n" << msg_box;
							} else {
								msg << pair.first << ": "
									<< **(pair.second.IntPtr)
									<< " [0x" << *(pair.second.IntPtr) << "]"
									<< "\r\n" << msg_box;
							}
							break;
						case PropDoublePtr :
							if (pair.second.DoublePtr == nullptr) {
								msg << pair.first << ": " << "n/a [0x" << pair.second.DoublePtr << "]\r\n" << msg_box;
							} else {
								msg << pair.first << ": "
									<< **(pair.second.DoublePtr)
									<< " [0x" << *(pair.second.DoublePtr) << "]"
									<< "\r\n" << msg_box;
							}
							break;
					}
				}
			}

	};

}