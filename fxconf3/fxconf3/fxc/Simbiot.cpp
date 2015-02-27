#pragma once

#include <algorithm>
#include <functional>
#include <unordered_map>

#include "../stdafx.h"
#include "../Defines.h"
#include "../Property.h"

#include "fxc.h"
#include "Parameters.cpp"
#include "Diller.cpp"

namespace fxc {
	
	class Simbiot :
		public Parameters, 
		public CPropertyList
	{

		public:

#pragma region Переменные и ссылки
	
			//Переменные менеджера ордеров и общей логики
			int		current_index;
			int		current_count;
			int		old_index;
			int		old_count;

			Diller*	dillers[2];
			Diller*	curdil;

			Order	current_orders[NUM_ORDERS];
			Order	old_orders[NUM_ORDERS];

			double	base_lot; //базовый лот
			double	equity;
			double	max_equity;
			//double	prev_indicator;
			//move_tp
			double	m_weight;
			double	m_last_weight;
			double	m_total_weight;
			int		m_index;
			bool	showend;
			double	profits[50];
			double	p_high;
			double  p_low;
			double  p_buy;
			double  p_sell;

			//Переменные для реализации индикаторов
			double*	closes;
			double*	highs;
			double* lows;
			double	tmAvr;
			double	tmBuffer[MAX_BARS];
			double	up_ind;
			double	dn_ind;
			double  wu, wd;
			double  prev_wu, prev_wd;
			double	wuBuffer[MAX_BARS];
			double	wdBuffer[MAX_BARS];
			double	maBuffer[3];
			int		counted;
			int		bars;
			double  tmup, tmdown;
			bool	first_calc;
			//bool	prev

#pragma endregion

			Simbiot(char* _symbol) {
				strcpy(symbol, _symbol);

				dillers[0] = new Diller(this, 0);
				dillers[1] = new Diller(this, 1);
				dillers[0]->opposite = dillers[1];  //Обмен ссылками друг на друга
				dillers[1]->opposite = dillers[0];

				registerProps();
			}

			void postInit() {
				printRedisteredProps();

				curdil      = dillers[0];
				current_count = 0;
				p_high      = 0;
				p_low       = 1000000;
				p_buy       = 0;
				p_sell      = 0;
				counted     = 0;

				m_index = 0;
				showend = true;

				dillers[0]->Reset();
				dillers[1]->Reset();

				for (int i = 0; i < 50; i++) {
					profits[i] = input_takeprofit * pow(input_pips_mult, i);
				}
				
				*ext_indicator            = 0;
				prev_indicator        = -50;
				first_calc            = true;
				dillers[0]->_base_lot = input_buy_lot;
				dillers[1]->_base_lot = input_sell_lot;
				curdil->cur_av_lvl    = input_av_lvl;
				base_lot              = curdil->_base_lot;
			}

			void sortOrders() {
				MARK_FUNC_IN

				if (_isSorted) {
					MARK_FUNC_OUT
					return;
				}

				sortDillerOrders(dillers[0], [](const Order* a, const Order* b) { return a->openprice > b->openprice; });
				sortDillerOrders(dillers[1], [](const Order* a, const Order* b) { return a->openprice < b->openprice; });

				curdil             = dillers[0];
				*ext_count_p       = dillers[0]->level;
				*(ext_count_p + 1) = dillers[1]->level;
				_isSorted          = true;

				MARK_FUNC_OUT
			}

#pragma region Checks

			//Проверка уровня стоплосса или тейкпрофита
			bool check_sl(int _type, double low_price, double high_price) { //Понятия high low для покупки
				return(!(low_price && high_price && dillers[_type % 2]->delta(low_price, high_price) < input_min_sl_tp));
			}

			//Проверка уровня заморозки
			bool check_freeze(double pr1, double pr2) {
				return(abs(pr1 - pr2) > input_freeze);
			}
			
			//Проверка на правильность параметров при открытии ордера
			int check_new(int type, double* lots, double* openprice, double* slprice, double* tpprice) {
				type = type % 2;
				if (
					type < 2 && // Для немедленного исполнения
					(
						!check_sl(type, *slprice, dillers[type]->mpc) ||
						!check_sl(type, dillers[type]->mpc, *tpprice)
					)
				) {
					ShowInfo("chk_new tp", *tpprice);
					ShowInfo("chk_new sl", *tpprice);
					ShowInfo("chk_new min", input_min_sl_tp);
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

				*lots      = normlot(*lots);
				*openprice = norm(*openprice);
				*tpprice   = norm(*tpprice);
				*slprice   = norm(*slprice);

				return 0;
			}
			
			//Проверка на правильность параметров при модификации ордера
			//int check_mod() {
			//	if (get_order(*ext_o_ticket) == 0) {
			//		return 200;
			//	}
			//
			//	*ext_o_openprice = norm(*ext_o_openprice);
			//	*ext_o_tpprice   = norm(*ext_o_tpprice);
			//	*ext_o_slprice   = norm(*ext_o_slprice);
			//
			//	int   _type = cur_order.type;
			//	double _mpc = dillers[_type]->mpc;
			//	double _mpo = dillers[_type]->mpo;
			//
			//	if (
			//		_type < 2 &&   //Для открытых ордеров
			//		(
			//			!check_freeze(cur_order.slprice, _mpc) ||
			//			!check_freeze(cur_order.tpprice, _mpc) ||
			//			!check_sl(_type, *ext_o_slprice, _mpc) ||
			//			!check_sl(_type, _mpc, *ext_o_tpprice)
			//		)
			//	) {
			//		return 201;
			//	}
			//
			//	if (
			//		_type > 1 &&  //Для отложенных ордеров
			//		(
			//			!check_freeze(cur_order.openprice, _mpo) ||
			//			!check_sl(_type, *ext_o_slprice, *ext_o_openprice) ||
			//			!check_sl(_type, *ext_o_openprice, *ext_o_tpprice)
			//		)
			//	) {
			//		return 201;
			//	}
			//
			//	if (
			//		(_type == OP_BUYLIMIT || _type == OP_SELLLIMIT) &&
			//		!check_sl(_type, cur_order.openprice, _mpo)
			//	) {
			//		return 202;
			//	}
			//
			//	if (
			//		(_type == OP_BUYSTOP || _type == OP_SELLSTOP) &&
			//		!check_sl(_type, _mpo, cur_order.openprice)
			//	) {
			//		return 202;
			//	}
			//
			//	return 0;
			//}

#pragma endregion

			//Нормализует лот
			double normlot(double value) {
				int steps = int(ceil(value / input_lot_step));
				value = steps * input_lot_step;
				value = max(value, input_lot_min);
				return min(value, input_lot_max);
			}

			//3.1 Инициализация цикла обновления ордеров, поскольку цикл получается рваным (каждая итерация вызывается из MQL), нельзя пользоватья результатами в MQL программе до завершения цикла
			virtual void refresh_init(double ask, double bid, double equity_) {
				_isSorted       = false;
				equity          = equity_;
				dillers[0]->mpo = dillers[1]->mpc = ask;
				dillers[1]->mpo = dillers[0]->mpc = bid;
				current_index   = 0; // Обнуляем общий счетчик ордеров

				if (!input_is_optimization) { // Если не оптимизация
					memcpy(old_orders, current_orders, sizeof(current_orders));  // список текущих ордеров, делаем старым
					old_count = current_count;
					old_index = 0;
				}

				dillers[0]->ResetTick(); // Сбрасываем сортировочные показатели по типам
				dillers[1]->ResetTick();

				if (!input_new_bar)
					calc_indicator();
			}

			//3.1 Добавляет новый ордер в цикле скана ордеров, в будущем возвращает код изменения
			int refresh_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0) {
				auto order = &current_orders[current_index];

				order->ticket    = _ticket;
				order->type      = _type;
				order->lots      = _lots;
				order->openprice = _openprice;
				order->tpprice   = _tp;
				order->slprice   = _sl;

				switch (_type) {
					case OP_BUY :
					case OP_SELL :
						order->profit          = _profit;
						*(ext_open_dd    + _type) += _profit;
						*(ext_total_lots + _type) += _lots;
						dillers[_type]->orders.push_back(order);
						break;

					case OP_BUYLIMIT :
					case OP_SELLLIMIT :
						dillers[_type % 2]->ord_limit = order;
						break;

					case OP_BUYSTOP :
					case OP_SELLSTOP :
						dillers[_type % 2]->ord_stop  = order;
						break;
				}

				current_count = ++current_index;

				return 0;
			}

			void refresh_prices(double *_closes, double *_highs, double *_lows, int _bars) {
				closes = _closes;
				highs  = _highs;
				lows   = _lows;
				bars   = _bars;
				calc_indicator();
			}
			
			// Выдает тикеты закрытых ордеров, пока они есть
			int getclosed() {
				bool   found;
				Order* order;
				
				while (old_index < old_count) {
					order = &old_orders[old_index++];

					if (order->type != OP_BUY && order->type != OP_SELL) { // Пропускаем не рыночные ордера
						continue;
					}

					found = false;
					for (int i = 0; i < current_count; i++) {
						if (order->ticket == current_orders[i].ticket) {
							found = true;
							break;
						}
					}

					if (!found) {
						return order->ticket;
					}
				}

				return 0;
			}

			void attachListener(const std::string eventType, const std::function<void()> &listener) {
				_listeners[eventType].push_back(listener);
			}

			void raiseEvent(const std::string eventType) {
				auto list = _listeners.find(eventType);
				if (list != _listeners.end()) {
					for each (auto listener in list->second) {
						listener();
					}
				}
			}

			template <class T> void ShowInfo(char text[], T value, bool show = false) {
#if DEBUG 
				if (!input_is_visual)
					return;

				if (showend) {
					fxc::msg << "Ask: " << dillers[0]->mpo << "   Bid: " << dillers[1]->mpo << "\r\n";
					LPCSTR s = (curdil->type) ? " Sell" : "  Buy";
					fxc::msg << "Type: " << curdil->type << s << " sorted=" << _isSorted << "\r\n";
					fxc::msg << "Count Buy: " << dillers[0]->level << " (" << dillers[0]->orders.size() << "), Sell: " << dillers[1]->level << "\r\n";
					//msg << "k=" << __step << " total: " << current_count << "\r\n";
					//fxc::msg << "o_type=" << *ext_o_type << ", o_lot=" << *ext_o_lots << "\r\n";
					//fxc::msg << "o_price=" << *ext_o_openprice << ", o_tp=" << *ext_o_tpprice << "\r\n";
					showend = false;
				}

				fxc::msg << text << ": " << value << "\r\n";
				if (show) {
					showend = true;
					fxc::msg << fxc::msg_box;
				}
#endif
			}

		private:

			bool _isSorted;
			std::unordered_map< std::string, std::vector< std::function<void()> > > _listeners;
			
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
				Register("max_lvl",     &ext_max_lvl);     // 104
				Register("max_dd",      &ext_max_dd);      // 105
				Register("indicator",   &ext_indicator);   // 106
				Register("count_p",     &ext_count_p);     // 107
				Register("indicator2",  &ext_indicator2);  // 116

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

			void printRedisteredProps() {
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

			void calc_indicator() {
				double main_avr;
				double diffup;
				double diffdn;
				double price = dillers[0]->mpo - dillers[0]->mpc;

				closes[input_buf_len] = dillers[0]->mpc;
				highs[input_buf_len]  = max(dillers[0]->mpc, highs[input_buf_len]);
				lows[input_buf_len]   = min(dillers[0]->mpc, lows[input_buf_len]);

				int start = min(bars - counted, input_periodf2);
				if (start == input_periodf2) {
					prev_wd = prev_wu = input_delta*input_delta;
				}

				for (int i = start; i >= 0; i--) {
					main_avr = GetMAW(input_period, i);
					diffup   = max(highs[input_buf_len - i] - main_avr, input_delta);
					diffdn   = max(main_avr - lows[input_buf_len - i], input_delta);
					diffup  *= diffup;
					diffdn  *= diffdn;

					wu = (prev_wu*(input_periodf2 - 1) + diffup) / input_periodf2;
					wd = (prev_wd*(input_periodf2 - 1) + diffdn) / input_periodf2;

					if (i > 0) {
						prev_wu = wu;
						prev_wd = wd;
					}
				}

				counted = bars;
				up_ind  = main_avr + input_deviation * sqrt(wu);
				dn_ind  = main_avr - input_deviation * sqrt(wd);

				if (input_periodf3) {
					for (int i = 1; i >= 0; i--) {
						maBuffer[i] = GetMAW(input_periodf3, i);
					}
				}
			}

			inline double GetMAW(int period, int shift) {
				int j, k;
				double sum  = (period + 1) * closes[input_buf_len - shift];
				double sumw = period + 1;

				for (j = 1, k = period; j <= period; j++, k--) {
					sum  += k * closes[input_buf_len - (shift + j)];
					sumw += k;
				}
				
				return sum / sumw;
			}

			//выдает ордер из менеджера по тикету
			inline bool get_order(int _ticket) {
				for (int i = 0; i < current_count; i++) {
					if (current_orders[i].ticket == _ticket) {
						cur_order = current_orders[i];
						return true;
					}
				}

				return false;
			}

			//Нормализует цену
			inline double norm(double value) {
				return(floor(value / input_point + 0.5) * input_point);
			}

			inline void sortDillerOrders(fxc::Diller* diller, std::function<bool (const Order* a, const Order* b)> comparer) {
				if (diller->level = diller->orders.size()) {
					std::sort(diller->orders.begin(), diller->orders.end(), comparer);
					diller->first = diller->orders[0];
					diller->last  = diller->orders.back();
				}
				else {
					diller->first = nullptr;
					diller->last  = nullptr;
				}
			}

	};

}