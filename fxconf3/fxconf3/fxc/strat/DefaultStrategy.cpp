#pragma once

#include "AbstractStrategy.cpp"

namespace fxc {

namespace strategy {

	class DefaultStrategy : public AbstractStrategy {

		public:

			DefaultStrategy(char* _symbol) : AbstractStrategy(_symbol) {}

			virtual int getJob() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::getJob()\r\n" << fxc::msg_box;
#endif
				return start();
			}

		protected:

			int start() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::start()\r\n" << fxc::msg_box;
#endif

				sortOrders();
				if (!curdil->level) { //Если нет ордеров в рынке
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::start() [1]\r\n" << fxc::msg_box;
#endif
					if (!input_stop_new[curdil->type] && (signal() || curdil->opposite->level >= input_forward_lvl)) { //Если не запрещено открытие новой сетки и есть сигнал, открываем первый ордер
#if TRACE_STRATEGY
						fxc::msg << "-> DefaultModule::start() [1.1]\r\n" << fxc::msg_box;
#endif
						calc_first_lot();

						if (calc_oposite()) { //Отрабатываем обратное усреднение
#if TRACE_STRATEGY
							fxc::msg << "-> DefaultModule::start() [1.1.1]\r\n" << fxc::msg_box;
#endif
							return JOB_MODIFY;
						}
						else {
#if TRACE_STRATEGY
							fxc::msg << "-> DefaultModule::start() [1.1.2]\r\n" << fxc::msg_box;
#endif
							return openOrder();
						}
					}
						
					return deleteOpposite();
				}
				else if (!input_stop_avr[curdil->type] && curdil->level < input_max_grid_lvl) { //Если есть базовый ордер и разрешено усреднять и максимальный уровень не достигнут
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::start() [2]\r\n" << fxc::msg_box;
#endif
					if (curdil->opposite->level >= input_forward_lvl && !curdil->ord_stop && calc_forward()) { //Если разрешены форварды и его можно поставить
#if TRACE_STRATEGY
						fxc::msg << "-> DefaultModule::start() [2.1]\r\n" << fxc::msg_box;
#endif
						return JOB_CREATE;
					}
					else {
#if TRACE_STRATEGY
						fxc::msg << "-> DefaultModule::start() [2.2]\r\n" << fxc::msg_box;
#endif
						return move(); //Наращивание ступеней сетки
					}
				}
				else { //Если есть ордера, но не разрешено усреднять -> удалить отложки
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::start() [3]\r\n" << fxc::msg_box;
#endif
					return deleteSelf();
				}
			}

			int openOrder() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::openOrder()\r\n" << fxc::msg_box;
#endif
				if (input_opp_close && close_profit()) {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::openOrder() [1]\r\n" << fxc::msg_box;
#endif
					return JOB_CLOSE;
				}
				else if (calc_first()) {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::openOrder() [2]\r\n" << fxc::msg_box;
#endif
					return JOB_CREATE; //Открыть первый ордер
				}
				else {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::openOrder() [3]\r\n" << fxc::msg_box;
#endif
					return deleteOpposite();
				}
			}

			int deleteOpposite() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::deleteOpposite()\r\n" << fxc::msg_box;
#endif
				if (curdil->opposite->ord_stop) {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::deleteOpposite() [1]\r\n" << fxc::msg_box;
#endif
					*ext_o_ticket = curdil->opposite->ord_stop->ticket;
					return JOB_DELETE; //Удалить стоп ордер
				}
				else {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::deleteOpposite() [2]\r\n" << fxc::msg_box;
#endif
					return deleteSelf();
				}
			}

			int deleteSelf() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::deleteSelf()\r\n" << fxc::msg_box;
#endif
				if (curdil->ord_stop) {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::deleteSelf() [1]\r\n" << fxc::msg_box;
#endif
					*ext_o_ticket = curdil->ord_stop->ticket;
					return JOB_DELETE; //Удалить стоп ордер
				}
				else {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::deleteSelf() [2]\r\n" << fxc::msg_box;
#endif
					return finalize();
				}
			}

			int move() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::move()\r\n" << fxc::msg_box;
#endif
				if (move_tp()) { //Если есть что двигать, двигаем
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::move() [1]\r\n" << fxc::msg_box;
#endif
					return JOB_MODIFY;
				}
				else if (signal() && calc_next()) { //Если можно выставить отложку или по рынку, выставляем
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::move() [2]\r\n" << fxc::msg_box;
#endif
					return JOB_CREATE;
				}
				else {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::move() [3]\r\n" << fxc::msg_box;
#endif
					return finalize();
				}
			}

			int preFinalize() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::preFinalize()\r\n" << fxc::msg_box;
#endif
				if (input_opp_close == 2 && close_profit()) {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::preFinalize() [1]\r\n" << fxc::msg_box;
#endif
					return JOB_CLOSE;
				}
				else {
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::preFinalize() [2]\r\n" << fxc::msg_box;
#endif
					return finalize();
				}
			}

			int finalize() {
#if TRACE_STRATEGY
				fxc::msg << "-> DefaultModule::finalize()\r\n" << fxc::msg_box;
#endif
				if (curdil->type == OP_BUY) { //Все сначала только для продажи
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::finalize() [1]\r\n" << fxc::msg_box;
#endif
					curdil = dillers[1];
					return start();
				}
				else { //Отрабатываем таймауты и завершаем алгоритм
#if TRACE_STRATEGY
					fxc::msg << "-> DefaultModule::finalize() [2]\r\n" << fxc::msg_box;
#endif
					curdil = dillers[0];
					prev_indicator = *ext_indicator;
					return JOB_EXIT;
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

			void calc_first_lot() {
				curdil->cur_av_lvl = input_av_lvl; //Восстанавливаем текущий уровень усреднения на максимум
				if (input_auto_mm > 0) { //Если включен манименеджмент
					base_lot = normlot(floor(equity / input_mm_equ) * input_lot_step);
					curdil->_base_lot = base_lot;
				}
				else {
					base_lot = curdil->_base_lot;
				}

				*ext_o_lots = base_lot;
				if (curdil->opposite->level >= input_trend_lvl) { //Отрабатываем увеличение при тренде
					*ext_o_lots = max(*ext_o_lots, base_lot * input_trend_lot_mult * pow(input_trend_progress, curdil->opposite->level - input_trend_lvl));
				}
				else if (curdil->prev_lvl >= input_repeat_lvl) { //Отрабатываем увеличение при повторе
					*ext_o_lots = max(*ext_o_lots, base_lot * input_repeat_lot_mult * pow(input_repeat_progress, curdil->prev_lvl - input_repeat_lvl));
				}

				if (curdil->opposite->last)
					*ext_o_lots = max(*ext_o_lots, curdil->opposite->last->lots * input_lot_hadge_mult); //Отрабатываем простое хэджирование

				if (input_weighthadge)
					*ext_o_lots = max(*ext_o_lots, (input_takeprofit * base_lot - curdil->opposite->basket_weight(*ext_o_tpprice, 100)*input_weighthadge) / input_takeprofit);

				*ext_o_lots = max(*ext_o_lots, *(ext_total_lots + curdil->opposite->type) * input_basket_hadge_mult);
				*ext_o_lots = max(*ext_o_lots, curdil->prev_lots * input_regres_mult); //Затухание
				*ext_o_lots = min(*ext_o_lots, input_maxlot);
			}
			
			bool calc_first() {
				*ext_o_openprice = curdil->mpo;
				if (curdil->level == 0) {
					*ext_o_tpprice = curdil->tp(*ext_o_openprice, input_takeprofit);
				}
				else {
					*ext_o_tpprice = curdil->tp(*ext_o_openprice, input_takeprofit);
				}

				*ext_o_type    = curdil->type;
				*ext_o_slprice = curdil->sl(*ext_o_openprice, input_stoploss);
				*ext_intret    = check_new();

				if (*ext_intret > 0) {
					ShowInfo("*** calc_first check error", *ext_intret);
					return false;
				}

				*ext_intret = 1;
				return true;
			}
			
			bool calc_forward() {
				*ext_o_openprice = curdil->tp(curdil->first->openprice, input_step * input_forward_step_mult);
				double d = curdil->delta(curdil->mpo, *ext_o_openprice);

				if (d > input_min_sl_tp) {   //если можно выставить отложку
					*ext_o_type = curdil->type + 4;
				}
				else if (d <= 0) { //если нельзя отложку, но можно по рынку
					*ext_o_openprice = curdil->mpo;
					*ext_o_type      = curdil->type;
				}
				else { //Пока не можем выставить не отложку не по рынку
					return false;
				}

				calc_first_lot();
				*ext_o_tpprice = curdil->tp(*ext_o_openprice, input_takeprofit);
				*ext_o_slprice = curdil->sl(*ext_o_openprice, input_stoploss);
				*ext_intret    = check_new();

				if (*ext_intret > 0) {
					ShowInfo("*** calc_forward check error", *ext_intret);
					return false;
				}

				return true;
			}
			
			bool calc_next() {
				*ext_o_openprice = curdil->sl(curdil->last->openprice, input_step);
				double d = curdil->delta(*ext_o_openprice, curdil->mpo);  //Расстояние до будущего ордера, если отрицательное, то проехали, надо ставить по рынку

				if (d <= 0) { //Выставляем по рынку
					*ext_o_openprice = curdil->mpo;
					*ext_o_type      = curdil->type;
				}
				else { //Пока не можем выставить не отложку не по рынку
					curdil->step_peak = 0;
					return false;
				}

				*ext_o_slprice = curdil->sl(*ext_o_openprice, input_stoploss);
				//Ограничение усреднения по размеру лота
				if (curdil->last->lots >= input_av_lot) {
					if (curdil->cur_av_lvl == input_av_lvl && curdil->level < input_av_lvl) {
						curdil->cur_av_lvl = max(1, curdil->level);
					}
					else {
						curdil->cur_av_lvl = max(1, curdil->cur_av_lvl - 1);
					}
				}

				*ext_o_tpprice = curdil->tp(*ext_o_openprice, input_takeprofit);
				double nextProfit = profits[curdil->level - 1];
				*ext_o_lots = (nextProfit * curdil->_base_lot - curdil->basket_weight(*ext_o_tpprice, curdil->cur_av_lvl)) / input_takeprofit;

				if (input_safe_copy) {
					*ext_o_lots = max(*ext_o_lots, curdil->_base_lot);
				}

				*ext_o_lots = min(*ext_o_lots, input_maxlot);
				*ext_intret = check_new();
				if (*ext_intret > 0) {
					ShowInfo("*** calc_next check error", *ext_intret);
					return false;
				}

				*ext_intret = curdil->level + 1;
				return true;
			}
			
			bool calc_oposite() {
				if (curdil->opposite->GetOrder() && abs(cur_order.openprice - curdil->mpo) > input_step * input_op_av_lvl) {
					double tp_price = curdil->tp(curdil->mpo, input_takeprofit);
					double oplots   = (input_takeprofit * base_lot - curdil->opposite->order_weight(cur_order.openprice, tp_price, cur_order.lots)) / input_takeprofit;
					
					if (oplots > input_maxlot)  //Если лот превышает максимум, то обратку не усредняем
						return false;

					*ext_o_lots      = max(base_lot, oplots); //Вместо base_lot было *ext_o_lots незнаю почему
					*ext_o_ticket    = cur_order.ticket;
					*ext_o_openprice = cur_order.openprice;
					*ext_o_slprice   = tp_price;
					*ext_o_tpprice   = cur_order.tpprice;
					*ext_intret      = check_mod();
					
					if (*ext_intret > 0)
						return false;

					return true;
				}
				return false;
			}

	};

}

}