#pragma once

#include "AbstractStrategy.cpp"
#include "../indicators/DefaultIndicator.cpp"

namespace fxc {

namespace strategy {

	class DefaultStrategy : public AbstractStrategy {

		public:

			DefaultStrategy(char* _symbol) : AbstractStrategy(_symbol) {
				indicator = new fxc::indicator::DefaultIndicator(this, this);
			}

			virtual int getJob() {
				MARK_FUNC_IN

				((TradeManager*) this)->reset();
				sortOrders();

				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];

					// Если нет ордеров в рынке
					if (!curdil->level) {
						// Если не запрещено открытие новой сетки и есть сигнал
						if (!input_stop_new[curdil->type] && (compSignal() || curdil->opposite->level >= input_forward_lvl)) {
							createOrder(
								curdil->type, 
								compFirstLot(curdil->tp(curdil->mpo, input_takeprofit)),
								curdil->mpo,
								curdil->sl(curdil->mpo, input_stoploss),
								curdil->tp(curdil->mpo, input_takeprofit)
							);

							if (input_opp_close) {
								autoClose();
							}
						}
					}
					else {
						moveTP();

						// Если есть базовый ордер и разрешено усреднять и максимальный уровень не достигнут
						if (!input_stop_avr[curdil->type] && curdil->level < input_max_grid_lvl) {

							// Если разрешены форварды и его можно поставить
							if (curdil->opposite->level >= input_forward_lvl && !curdil->ord_stop) {
								tryOpenForward();
							}
							if (compSignal()) {
								openNextOrder();

								if (input_opp_close == 2) {
									autoClose();
								}
							}
						}
						// Если есть ордера, но не разрешено усреднять -> удалить отложки
						else {
							delStopLimitOrders();
						}
					}
				}

				MARK_FUNC_OUT
				return getActionsStackSize();
			}

			virtual void refresh_init(double _ask, double _bid, double _equity) {
				equity = _equity;
				AbstractStrategy::refresh_init(_ask, _bid, _equity);

				if (!input_new_bar) {
					indicator->compute();
				}
			}

			virtual void refresh_prices(double *closes, double *highs, double *lows, int bars) {
				AbstractStrategy::refresh_prices(closes, highs, lows, bars);
				indicator->compute();
			}

		protected:

			inline void moveTP() {
				MARK_FUNC_IN
				double last_tpprice = curdil->tp(curdil->last->openprice, input_takeprofit);

				// Если у последнего ордера еще не установлен тейкпрофит, то ставим его
				if (abs(curdil->last->tpprice - last_tpprice) > input_point) {
					modOrder(
						curdil->last->ticket, 
						curdil->last->openprice, 
						curdil->sl(curdil->last->openprice, input_stoploss), 
						last_tpprice
					);
				}

				// Если в рынке один ордер, то нечего и двигать
				if (curdil->level < 2) {
					MARK_FUNC_OUT
					return;
				}

				double last_weigth  = curdil->orderWeight(curdil->last->openprice, last_tpprice, curdil->last->lots);
				double total_weight = curdil->basketWeight(last_tpprice);

				// Если вес всей сетки положителен, то удесятеряем вес последнего ордера для гарантированного закрытия всей сетки
				if (total_weight > 0) {
					last_weigth *= 10;
				}

				for (int i = 0; i < curdil->level - 1; i++) {
					last_weigth -= curdil->orderWeight(curdil->orders[i]->openprice, last_tpprice, curdil->orders[i]->lots);

					if (last_weigth > 0 && abs(curdil->orders[i]->tpprice - last_tpprice) > input_point) {
						modOrder(
							curdil->orders[i]->ticket, 
							curdil->orders[i]->openprice, 
							curdil->sl(curdil->orders[i]->openprice, input_stoploss), 
							last_tpprice
						);
					}
				}
				MARK_FUNC_OUT
			}

			double compFirstLot(double tpprice) {
				MARK_FUNC_IN
				//Восстанавливаем текущий уровень усреднения на максимум
				curdil->cur_av_lvl = input_av_lvl;
				
				double base_lot;

				//Если включен манименеджмент
				if (input_auto_mm > 0) {
					base_lot = normLot(floor(equity / input_mm_equ) * input_lot_step);
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
					result = max(result, (input_takeprofit * base_lot - curdil->opposite->basketWeight(tpprice, 100) * input_weighthadge) / input_takeprofit);

				result = max(result, *(ext_total_lots + curdil->opposite->type) * input_basket_hadge_mult);
				result = max(result, curdil->prev_lots * input_regres_mult); //Затухание
				result = min(result, input_maxlot);

				MARK_FUNC_OUT
				return result;
			}

			bool compSignal() {
				MARK_FUNC_IN

				//Первый ордер открываем в любом случае
				if (input_first_free && curdil->level == 0) {
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->level >= input_free_lvl) {
					MARK_FUNC_OUT
					return true;
				}

				auto buffer = indicator->getMaBuffer();

				if (curdil->type) { //Продажи
					if (curdil->step_peak) {
						if (
							(input_periodf3 && *buffer < *(buffer+1)) ||
							(input_periodf3 == 0 && curdil->step_peak - curdil->mpo >= input_rollback) ||
							(input_periodf3 == 0 && input_rollback == 0)
						) {
							curdil->step_peak = 0;
							MARK_FUNC_OUT
							return true;
						}
						else {
							curdil->step_peak = max(curdil->step_peak, curdil->mpo);
						}

					}
					else if (curdil->mpo > *indicator->getUp()) {
						curdil->step_peak = curdil->mpo;
					}
				}
				else { //Покупки
					if (curdil->step_peak) {
						if (
							(input_periodf3 && *buffer > *(buffer+1)) ||
							(input_periodf3 == 0 && curdil->mpo - curdil->step_peak >= input_rollback) ||
							(input_periodf3 == 0 && input_rollback == 0)
						) {
							curdil->step_peak = 0;
							MARK_FUNC_OUT
							return true;
						}
						else {
							curdil->step_peak = min(curdil->step_peak, curdil->mpo);
						}
					}
					else if (curdil->mpo < *indicator->getDown()) {
						curdil->step_peak = curdil->mpo;
					}
				}

				MARK_FUNC_OUT
				return false;
			}

		private:

			double	equity;
			Diller* curdil;
			fxc::indicator::DefaultIndicator* indicator;

			inline void tryOpenForward() {
				MARK_FUNC_IN
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
						MARK_FUNC_OUT
						return;
					}

					createOrder(
						o_type, 
						compFirstLot(curdil->tp(curdil->mpo, input_takeprofit)),
						openprice, 
						curdil->sl(openprice, input_stoploss), 
						curdil->tp(openprice, input_takeprofit)
					);
				}
				MARK_FUNC_OUT
			}

			inline void openNextOrder() {
				MARK_FUNC_IN
				double openprice = curdil->mpo;

				//Расстояние до будущего ордера, если отрицательное, то проехали
				if (curdil->delta(curdil->sl(curdil->last->openprice, input_step), openprice) > 0) {
					//Пока не можем выставить не отложку не по рынку
					curdil->step_peak = 0;
					MARK_FUNC_OUT
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
				double lots       = (nextProfit * curdil->_base_lot - curdil->basketWeight(tpprice, curdil->cur_av_lvl)) / input_takeprofit;

				if (input_safe_copy) {
					lots = max(lots, curdil->_base_lot);
				}

				lots = min(lots, input_maxlot);

				if (0 == check_new(curdil->type, &lots, &openprice, &slprice, &tpprice)) {
					createOrder(curdil->type, lots, openprice, slprice, tpprice);
				}
				MARK_FUNC_OUT
			}

			inline void autoClose() {
				MARK_FUNC_IN

				if (curdil->opposite->basketCost() > 0) {
					for (int i = 0; i < curdil->opposite->level; i++) {
						closeOrder(
							curdil->opposite->orders[i]->ticket,
							curdil->opposite->orders[i]->lots,
							curdil->opposite->mpc
						);
					}
				}
				MARK_FUNC_OUT
			}

			inline void delStopLimitOrders() {
				MARK_FUNC_IN
				if (curdil->ord_stop) {
					deleteOrder(curdil->ord_stop->ticket);
				}
				MARK_FUNC_OUT
			}

	};

}

}