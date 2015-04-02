#pragma once

#include <functional>
#include <algorithm>

#include "debug/Debug.h"
#include "Parameters.cpp"
#include "ChartData.cpp"
#include "Diller.cpp"

namespace fxc {

	class OrdersManager {

		public:

			OrdersManager() {
				MARK_FUNC_IN
				dillers[0] = new Diller(OP_BUY);
				dillers[1] = new Diller(OP_SELL);

				dillers[0]->opposite = dillers[1];
				dillers[1]->opposite = dillers[0];
				MARK_FUNC_OUT
			}
			
			void init(Parameters* params) {
				for (int i = 0, l = sizeof(profits) / sizeof(*profits); i < l; i++) {
					profits[i] = params->input_takeprofit * pow(params->input_pips_mult, i);
				}

				if (!params->input_is_optimization) { // Если не оптимизация
					copyOrders = [&]() {
						old_index  = 0;
						old_length = current_index;
						memcpy(old_orders, current_orders, old_length); // список текущих ордеров, делаем старым
					};
				}
				else {
					copyOrders = []() {};
				}

				dillers[0]->cur_av_lvl = params->input_av_lvl;
				dillers[1]->cur_av_lvl = params->input_av_lvl;
				dillers[0]->_base_lot  = params->input_buy_lot;
				dillers[1]->_base_lot  = params->input_sell_lot;
			}

			void sortOrders() {
				MARK_FUNC_IN

				if (isSorted) {
					MARK_FUNC_OUT
					return;
				}

				dillers[0]->sortOrders();
				dillers[1]->sortOrders();

				*ext_count_p       = dillers[0]->level;
				*(ext_count_p + 1) = dillers[1]->level;

				isSorted = true;
				MARK_FUNC_OUT
			}

			void reset() {
				copyOrders();
				isSorted      = false;
				current_index = 0;

				for (auto type = 0; type < 2; type++) {
					*(ext_open_dd    + type) = 0.0;
					*(ext_total_lots + type) = 0.0;
					dillers[type]->reset();
				}
			}

			//3.1 Добавляет новый ордер в цикле скана ордеров, в будущем возвращает код изменения
			int addOrder(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0) {
				auto order = &current_orders[current_index++];

				order->ticket    = _ticket;
				order->type      = _type;
				order->lots      = _lots;
				order->openprice = _openprice;
				order->tpprice   = _tp;
				order->slprice   = _sl;

				switch (_type) {
					case OP_BUY:
					case OP_SELL:
						order->profit              = _profit;
						*(ext_open_dd    + _type) += _profit;
						*(ext_total_lots + _type) += _lots;
						dillers[_type]->addOrder(order);
						break;

					case OP_BUYLIMIT:
					case OP_SELLLIMIT:
						dillers[_type % 2]->ord_limit = order;
						break;

					case OP_BUYSTOP:
					case OP_SELLSTOP:
						dillers[_type % 2]->ord_stop  = order;
						break;
				}

				return 0;
			}

			fxc::Diller* const getDillers() {
				return *dillers;
			}

			fxc::TimeSeries* const getTimeseries() {
				return timeseries;
			}

			fxc::ChartData* const getChartData(const int timeframe) {
				return timeseries->getChartData(timeframe);
			}

			// Выдает тикеты закрытых ордеров, пока они есть
			const int getNextClosedTicket() {
				bool   found;
				Order* order;

				//fxc::msg << "-> getNextClosedTicket(" << old_index << " / " << old_length << ")\r\n" << fxc::msg_box;
				while (old_index < old_length) {
					order = &old_orders[old_index++];
					//fxc::msg << "-> getNextClosedTicket::order [0x" << order << "]\r\n" << fxc::msg_box;


					if (order->type != OP_BUY && order->type != OP_SELL) { // Пропускаем не рыночные ордера
						continue;
					}

					found = false;
					for (int i = 0; i <= current_index; i++) {
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

		protected:

			Diller*     dillers[2];
			TimeSeries* timeseries = new TimeSeries();

			double	profits[50];

			double*	ext_open_dd;     //102 подсчет открытой просадки по каждому типу отдельно
			double*	ext_total_lots;  //103 суммарный объем лотов по каждому типу
			int*    ext_count_p;     //107

		private:

			bool isSorted = false;
			std::function<void()> copyOrders;

			int		old_index     = 0;
			int		old_length    = 0;
			int		current_index = 0;

			Order	old_orders[NUM_ORDERS];
			Order	current_orders[NUM_ORDERS];

	};

}