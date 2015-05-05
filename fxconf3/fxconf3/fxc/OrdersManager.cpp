#pragma once

#include <functional>
#include <algorithm>

#include "debug/Debug.h"
#include "TerminalInfo.cpp"
#include "Diller.cpp"

namespace fxc { 

	class OrdersManager :
		public TerminalInfo	{

		public:

			OrdersManager() :
				TerminalInfo(){
				MARK_FUNC_IN
				dillers[0] = new Diller(OP_BUY);
				dillers[1] = new Diller(OP_SELL);

				dillers[OP_BUY]->opposite  = dillers[OP_SELL];
				dillers[OP_SELL]->opposite = dillers[OP_BUY];
				MARK_FUNC_OUT
			}
			~OrdersManager() {
				MARK_FUNC_IN
				delete dillers[0];
				delete dillers[1];
				//msg << "OrdersManager: delete dillers\r\n" << msg_box;
				MARK_FUNC_OUT
			}
			virtual void onOrderClose(int ticket) {};

			void initOrdersManager() {
				terminalInfoCalc();
				if (!mqlOptimization) { // ≈сли не оптимизаци€
					copyOrders = [&]() {
						old_length = current_length;
						memcpy(old_orders, current_orders, old_length); // список текущих ордеров, делаем старым
					};
				}
				else {
					copyOrders = []() {};
				}
				// ѕредварительна€ инициализаци€ базового лота, после получени€ параметров советника будет переопределение
				dillers[OP_BUY]->base_lot  = symbolMinLot; 
				dillers[OP_SELL]->base_lot = symbolMinLot; 
			}

			inline void sortOrders() {
				MARK_FUNC_IN
				if (isSorted) {
					MARK_FUNC_OUT
					return;
				}
				calcClosedOrders();
				dillers[0]->sortOrders();
				dillers[1]->sortOrders();
				isSorted = true;
				MARK_FUNC_OUT
			}

			void resetOrderManager() {
				MARK_FUNC_IN
				copyOrders();
				isSorted       = false;
				current_length = 0;
				dillers[OP_BUY]->reset();
				dillers[OP_SELL]->reset();
				MARK_FUNC_OUT
			}

			//3.1 ƒобавл€ет новый ордер в цикле скана ордеров, в будущем возвращает код изменени€
			int addOrder(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0) {
				auto order = &current_orders[current_length++];
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

			//¬ыдает событи onOrderClose  дл€ закрытых ордеров
			inline void calcClosedOrders() {
				bool   found;
				Order* order;
				for (int old_index = 0; old_index < old_length; old_index++) {
					order = &old_orders[old_index];
					if (order->type != OP_BUY && order->type != OP_SELL) { // ѕропускаем не рыночные ордера
						continue;
					}
					found = false;
					for (int cur_index = 0; cur_index < current_length; cur_index++) {
						if (order->ticket == current_orders[cur_index].ticket) {
							found = true;
							break;
						}
					}
					if (!found) {
						onOrderClose(order->ticket);
					}
				}
			}

		protected:

			Diller* dillers[2];

		private:

			bool isSorted = false;
			std::function<void()> copyOrders;

			//int		old_index     = 0;
			int		old_length    = 0;
			int		current_length = 0;

			Order	old_orders[NUM_ORDERS];
			Order	current_orders[NUM_ORDERS];

	};

}