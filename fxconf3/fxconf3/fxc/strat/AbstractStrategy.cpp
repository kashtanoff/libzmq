#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace strategy {

	class AbstractStrategy : public fxc::Simbiot {

		public:

			AbstractStrategy(char* _symbol) : Simbiot(_symbol) {}

			virtual int getJob() = 0;

		protected:

			bool move_tp() {
				if (curdil->last->tpprice == 0) {
					*ext_o_ticket    = curdil->last->ticket;
					*ext_o_openprice = curdil->last->openprice;
					*ext_o_tpprice   = curdil->tp(*ext_o_openprice, input_takeprofit);
					*ext_o_slprice   = curdil->sl(*ext_o_openprice, input_stoploss);
					*ext_intret      = check_mod();

					if (*ext_intret > 0) {
						ShowInfo("move_tp check_mod error", *ext_intret, true);
						return false;
					}

					return true;
				}
				
				if (curdil->level < 2) { //Если в рынке один ордер, то нечего и двигать
					return false;
				}

				if (curdil->order_weight(curdil->first->openprice, curdil->mpc, 1) > 0) {
					return false;
				}

				double last_tp = curdil->last->tpprice;
				if (m_index == 0) {
					m_weight       = 0.0;
					m_last_weight  = curdil->order_weight(curdil->last->openprice, last_tp, curdil->last->lots);
					m_total_weight = curdil->basket_weight(last_tp);
				}

				if (m_index < curdil->level - 1) {
					if (!curdil->GetOrder(m_index)) {
						m_index = 0;
						return false;
					}

					if (m_total_weight < 0.0) {
						m_weight += curdil->order_weight(cur_order.openprice, last_tp, cur_order.lots);
						if (m_last_weight + m_weight <= 0.0) {
							m_index = 0;
							return false;
						}
					}

					if (abs(cur_order.tpprice - last_tp) > input_point) {
						m_index++;
						*ext_o_ticket    = cur_order.ticket;
						*ext_o_openprice = cur_order.openprice;
						*ext_o_tpprice   = last_tp;
						*ext_o_slprice   = cur_order.slprice;
						*ext_intret      = check_mod();

						if (*ext_intret > 0) {
							ShowInfo("move_tp check_mod error", *ext_intret, true);
							return false;
						}

						return true;
					}
				}

				m_index = 0;
				return false;
			}

			bool close_profit() {
				if (c_index < 0 && curdil->opposite->basket_cost() > 0) { //Инициализация
					c_index = curdil->opposite->level;
				}
				
				if (c_index > 0) { //Если есть что закрывать - закрываем
					c_index--;
					*ext_o_ticket    = curdil->opposite->orders[c_index]->ticket;
					*ext_o_lots      = curdil->opposite->orders[c_index]->lots;
					*ext_o_openprice = curdil->opposite->mpc;
					return true;
				}

				c_index = -1;
				return false;
			}

	};

}

}