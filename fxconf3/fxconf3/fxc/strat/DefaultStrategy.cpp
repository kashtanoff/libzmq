#pragma once

#include "AbstractStrategy.cpp"

namespace fxc {

namespace strategy {

	class DefaultStrategy : public AbstractStrategy {

		public:

			DefaultStrategy(char* _symbol) : AbstractStrategy(_symbol) {}

			virtual int getJob() {
				actionsLen = 0;
				execJob();
				return actionsLen;
			}

			inline void execJob() {
				sortOrders();

				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];

					// Если нет ордеров в рынке
					if (!curdil->level) {
						// Если не запрещено открытие новой сетки и есть сигнал
						if (!input_stop_new[curdil->type] && (signal() || curdil->opposite->level >= input_forward_lvl)) {
							CreateOrder(
								curdil->type, 
								calc_first_lot(curdil->tp(curdil->mpo, input_takeprofit)), 
								curdil->mpo,
								curdil->sl(curdil->mpo, input_stoploss),
								curdil->tp(curdil->mpo, input_takeprofit)
							);

							if (input_opp_close) {
								AutoClose();
							}
						}
					}
					else {
						move_tp();

						// Если есть базовый ордер и разрешено усреднять и максимальный уровень не достигнут
						if (!input_stop_avr[curdil->type] && curdil->level < input_max_grid_lvl) {

							// Если разрешены форварды и его можно поставить
							if (curdil->opposite->level >= input_forward_lvl && !curdil->ord_stop) {
								TryOpenForward();
			
							}
							if (signal()) {
								OpenNextOrder();

								if (input_opp_close == 2) {
									AutoClose();
								}
							}
						}
						// Если есть ордера, но не разрешено усреднять -> удалить отложки
						else {
							DelStopLimitOrders();
						}
					}
				}
			}

		protected:

			int actionsLen;


			inline void move_tp() {
				//fxc::msg << " -> move_tp()\r\n" << fxc::msg_box;

				double last_tpprice = curdil->tp(curdil->last->openprice, input_takeprofit);

				if (curdil->last->tpprice != last_tpprice) { //Если у последнего ордера еще не установлен тейкпрофит, то ставим его
					ModOrder(
						curdil->last->ticket, 
						curdil->last->openprice, 
						curdil->sl(curdil->last->openprice, input_stoploss), 
						last_tpprice
					);
				}

				if (curdil->level < 2) //Если в рынке один ордер, то нечего и двигать
					return;

				double last_weigth = curdil->order_weight(curdil->last->openprice, last_tpprice, curdil->last->lots);
				double total_weight = curdil->basket_weight(last_tpprice);

				if (total_weight > 0) { //Если вес всей сетки положителен, то удесятеряем вес последнего ордера для гарантированного закрытия всей сетки
					last_weigth *= 10;
				}

				for (int i = 0; i < curdil->level - 1; i++) {
					last_weigth -= curdil->order_weight(curdil->orders[i]->openprice, last_tpprice, curdil->orders[i]->lots);

					if (last_weigth > 0 && curdil->orders[i]->tpprice != last_tpprice) {
						ModOrder(
							curdil->orders[i]->ticket, 
							curdil->orders[i]->openprice, 
							curdil->sl(curdil->orders[i]->openprice, input_stoploss), 
							last_tpprice
						);
					}
				}
			}


			bool signal() {
				if (input_first_free && curdil->level == 0)  //Первый ордер открываем в любом случае
					return true;

				if (curdil->level >= input_free_lvl)
					return true;

				if (curdil->type) { //Продажи
					if (curdil->step_peak) {
						if (
							(input_periodf3 && maBuffer[0] < maBuffer[1]) ||
							(input_periodf3 == 0 && curdil->step_peak - curdil->mpo >= input_rollback) ||
							(input_periodf3 == 0 && input_rollback == 0)
						) {
							curdil->step_peak = 0;
							return true;
						}
						else {
							curdil->step_peak = max(curdil->step_peak, curdil->mpo);
						}

					}
					else if (curdil->mpo > up_ind) {
						curdil->step_peak = curdil->mpo;
					}
				}
				else { //Покупки
					if (curdil->step_peak) {
						if (
							(input_periodf3 && maBuffer[0] > maBuffer[1]) ||
							(input_periodf3 == 0 && curdil->mpo - curdil->step_peak >= input_rollback) ||
							(input_periodf3 == 0 && input_rollback == 0)
						) {
							curdil->step_peak = 0;
							return true;
						}
						else {
							curdil->step_peak = min(curdil->step_peak, curdil->mpo);
						}
					}
					else if (curdil->mpo < dn_ind) {
						curdil->step_peak = curdil->mpo;
					}
				}

				return false;
			}

			double calc_first_lot(double tpprice) {
				curdil->cur_av_lvl = input_av_lvl; //Восстанавливаем текущий уровень усреднения на максимум

				if (input_auto_mm > 0) { //Если включен манименеджмент
					base_lot = normlot(floor(equity / input_mm_equ) * input_lot_step);
					curdil->_base_lot = base_lot;
				}
				else {
					base_lot = curdil->_base_lot;
				}

				double result = base_lot;

				//Отрабатываем увеличение при тренде
				if (curdil->opposite->level >= input_trend_lvl) {
					result = max(result, base_lot * input_trend_lot_mult * pow(input_trend_progress, curdil->opposite->level - input_trend_lvl));
				}
				//Отрабатываем увеличение при повторе
				else if (curdil->prev_lvl >= input_repeat_lvl) {
					result = max(result, base_lot * input_repeat_lot_mult * pow(input_repeat_progress, curdil->prev_lvl - input_repeat_lvl));
				}

				//Отрабатываем простое хэджирование
				if (curdil->opposite->last)
					result = max(result, curdil->opposite->last->lots * input_lot_hadge_mult);

				if (input_weighthadge)
					result = max(result, (input_takeprofit * base_lot - curdil->opposite->basket_weight(tpprice, 100) * input_weighthadge) / input_takeprofit);
				
				result = max(result, *(ext_total_lots + curdil->opposite->type) * input_basket_hadge_mult);
				result = max(result, curdil->prev_lots * input_regres_mult);  //Затухание
				result = min(result, input_maxlot);

				return result;
			}


			inline void TryOpenForward() {
				if (curdil->opposite->level >= input_forward_lvl && !curdil->ord_stop) {
					int    o_type;
					double openprice = curdil->tp(curdil->first->openprice, input_step * input_forward_step_mult);
					double d = curdil->delta(curdil->mpo, openprice);
					
					// если можно выставить отложку
					if (d > input_min_sl_tp) {
						o_type = curdil->type + 4;
					}
					// если нельзя отложку, но можно по рынку
					else if (d <= 0) {
						openprice = curdil->mpo;
						o_type    = curdil->type;
					}
					// Пока не можем выставить не отложку не по рынку
					else {
						return;
					}

					CreateOrder(
						o_type, 
						calc_first_lot(curdil->tp(curdil->mpo, input_takeprofit)),
						openprice, 
						curdil->sl(openprice, input_stoploss), 
						curdil->tp(openprice, input_takeprofit)
					);
				}
			}

			inline void OpenNextOrder() {
				double openprice  = curdil->mpo;

				//Расстояние до будущего ордера, если отрицательное, то проехали
				if (curdil->delta(curdil->sl(curdil->last->openprice, input_step), openprice) < 0) {
					//Пока не можем выставить не отложку не по рынку
					curdil->step_peak = 0;
					return;
				}

				//Ограничение усреднения по размеру лота
				if (curdil->last->lots >= input_av_lot) {
					if (curdil->cur_av_lvl == input_av_lvl && curdil->level < input_av_lvl) {
						curdil->cur_av_lvl = max(1, curdil->level);
					}
					else {
						curdil->cur_av_lvl = max(1, curdil->cur_av_lvl - 1);
					}
				}

				double slprice    = curdil->sl(openprice, input_stoploss);
				double tpprice    = curdil->tp(openprice, input_takeprofit);
				double nextProfit = profits[curdil->level - 1];
				double lots       = (nextProfit * curdil->_base_lot - curdil->basket_weight(tpprice, curdil->cur_av_lvl)) / input_takeprofit;

				if (input_safe_copy) {
					lots = max(lots, curdil->_base_lot);
				}

				lots = min(lots, input_maxlot);

				if (0 == check_new(curdil->type, &lots, &openprice, &slprice, &tpprice)) {
					CreateOrder(curdil->type, lots, openprice, slprice, tpprice);
				}
			}

			inline void AutoClose() {
				if (curdil->opposite->basket_cost() > 0) {
					for (int i = 0; i < curdil->opposite->level; i++) {
						CloseOrder(curdil->opposite->orders[i]->ticket, curdil->opposite->mpc, curdil->opposite->orders[c_index]->lots);
					}
				}
			}

			inline void DelStopLimitOrders() {
				if (curdil->ord_stop) {
					DeleteOrder(curdil->ord_stop->ticket);
				}
			}

			
			inline void CreateOrder(int type, double lots, double openprice, double slprice, double tpprice, std::string comment = "") {
				auto action         = ext_tradeActions[actionsLen++];
				action->o_type      = type;
				action->o_lots      = lots;
				action->o_openprice = openprice;
				action->o_slprice   = slprice;
				action->o_tpprice   = tpprice;
				action->actionId    = JOB_CREATE;
			}

			inline void ModOrder(int ticket, double openprice, double slprice, double tpprice) {
				//fxc::msg << " -> ModOrder(" << ticket << ", " << openprice << ", " << slprice << ", " << tpprice << ")\r\n" << fxc::msg_box;
				auto action         = ext_tradeActions[actionsLen++];
				action->o_ticket    = ticket;
				action->o_openprice = openprice;
				action->o_slprice   = slprice;
				action->o_tpprice   = tpprice;
				action->actionId    = JOB_MODIFY;
			}

			inline void DeleteOrder(int ticket) {
				auto action      = ext_tradeActions[actionsLen++];
				action->o_ticket = ticket;
				action->actionId = JOB_DELETE;
			}

			inline void CloseOrder(int ticket, double lots, double openprice) {
				auto action         = ext_tradeActions[actionsLen++];
				action->o_ticket    = ticket;
				action->o_lots      = lots;
				action->o_openprice = openprice;
				action->actionId    = JOB_CLOSE;
			}

	};

}

}