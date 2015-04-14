#pragma once

#include <functional>
#include <algorithm>

#include "debug/Debug.h"
#include "TerminalInfo.cpp"
#include "Diller.cpp"

namespace fxc { 

	class OrdersManager {

		public:

			OrdersManager() {
				MARK_FUNC_IN
				dillers[0] = new Diller(OP_BUY);
				dillers[1] = new Diller(OP_SELL);

				dillers[OP_BUY]->opposite = dillers[OP_SELL];
				dillers[OP_SELL]->opposite = dillers[OP_BUY];
				MARK_FUNC_OUT
			}
			
			void initOrdersManager(TerminalInfo* terminal) {
				if (!terminal->mqlOptimization) { // ≈сли не оптимизаци€
					copyOrders = [&]() {
						old_index  = 0;
						old_length = current_index;
						memcpy(old_orders, current_orders, old_length); // список текущих ордеров, делаем старым
					};
				}
				else {
					copyOrders = []() {};
				}
				//ѕредварительна€ инициализаци€ базового лота, после получени€ параметров советника будет переопределение
				dillers[OP_BUY]->base_lot = terminal->symbolMinLot; 
				dillers[OP_SELL]->base_lot = terminal->symbolMinLot; 
			}

			inline void sortOrders() {
				MARK_FUNC_IN
				if (isSorted) {
					return;
				}
				dillers[0]->sortOrders();
				dillers[1]->sortOrders();
				isSorted = true;
				MARK_FUNC_OUT
			}

			void reset() {
				copyOrders();
				isSorted      = false;
				current_index = 0;
				dillers[OP_BUY]->reset();
				dillers[OP_SELL]->reset();
			}

			//3.1 ƒобавл€ет новый ордер в цикле скана ордеров, в будущем возвращает код изменени€
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

			inline fxc::Diller* const getDillers() {
				return *dillers;
			}
			
			// ¬ыдает тикеты закрытых ордеров, пока они есть
			const int getNextClosedTicket() {
				bool   found;
				Order* order;

				//fxc::msg << "-> getNextClosedTicket(" << old_index << " / " << old_length << ")\r\n" << fxc::msg_box;
				while (old_index < old_length) {
					order = &old_orders[old_index++];
					//fxc::msg << "-> getNextClosedTicket::order [0x" << order << "]\r\n" << fxc::msg_box;


					if (order->type != OP_BUY && order->type != OP_SELL) { // ѕропускаем не рыночные ордера
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