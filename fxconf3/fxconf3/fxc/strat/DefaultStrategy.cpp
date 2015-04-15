#pragma once

#include "../fxc.h"
#include "../debug/Debug.h"
#include "AbstractStrategy.cpp"
#include "../debug/Debug.h"
#include "../indicators/RAIndicator.cpp"
#include "../Parameters.cpp"

namespace fxc {

namespace strategy {

	class DefaultStrategy : 
		public AbstractStrategy,
		public Parameters 
	{

		public:

			DefaultStrategy() :  
				AbstractStrategy(),
				Parameters((CPropertyList*) this){

			}

			virtual void initStrategy() {
				MARK_FUNC_IN
				paramsDeltaCalc(k_point * symbolPoint);
				for (int i = 0, l = sizeof(profits) / sizeof(*profits); i < l; i++) {
					profits[i] = deltaTP * pow(inputPipsMultiplier, i);
				}
				indicator = new fxc::indicator::RAIndicator(this, inputTimeFrame, inputPeriod1, inputPeriod2, inputDeviation, deltaMinDev);
				dillers[OP_BUY]->base_lot	= inputBaseLot[OP_BUY];
				dillers[OP_SELL]->base_lot	= inputBaseLot[OP_SELL];
				MARK_FUNC_OUT
			}

		protected:

			virtual void Strategy() {
				MARK_FUNC_IN
					
					// Если нет ордеров в рынке
					showValue(8, "Strategy working...");
				showValue(7, "new bars:", (int)getChartData(30)->newBars);
				if (!curdil->level) {  //Если ордеров нет, то если можно, открываем первый
					if (softBreak) {
						hardBreak = true;  //После завершения усреднения, включаем полный запрет
						status = "Averaging done. Trading stopped.";
						return;
					}

					if (!inputStopNew[curdil->type] && compSignal()) {
						createOrder(
							curdil->type,
							compFirstLot(),
							curdil->mpo,
							curdil->sl(curdil->mpo, deltaSL),
							curdil->tp(curdil->mpo, deltaFirstTP)
						);

						if (inputCloseMode) {
							autoClose();
						}
					}
				}
				else {
					moveTP();

					// Если есть базовый ордер и разрешено усреднять и максимальный уровень не достигнут
					if (!inputStop[curdil->type] && curdil->level < inputMaxGridLevel) {
						if (compSignal()) {
							openNextOrder();
							if (inputCloseMode == 2) {
								autoClose();
							}
						}
					}
					// Если есть ордера, но не разрешено усреднять -> удалить отложки
					else {
						delStopLimitOrders();
					}
				}
				MARK_FUNC_OUT
			}

			inline void moveTP() {
				MARK_FUNC_IN
				double tp = (curdil->level == 1) ? deltaFirstTP : deltaTP;
				double last_tpprice = curdil->tp(curdil->last->openprice, tp);
				double min_delta = max(symbolPoint, deltaStopLevel);

				// Если у последнего ордера еще не установлен тейкпрофит, то ставим его
				if (abs(curdil->last->tpprice - last_tpprice) > min_delta) {
					modOrder(
						curdil->last->ticket, 
						curdil->last->openprice, 
						curdil->sl(curdil->last->openprice, deltaSL),
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

					if (last_weigth > 0 && abs(curdil->orders[i]->tpprice - last_tpprice) > min_delta) {
						modOrder(
							curdil->orders[i]->ticket, 
							curdil->orders[i]->openprice, 
							curdil->sl(curdil->orders[i]->openprice, deltaSL),
							last_tpprice
						);
					}
				}
				MARK_FUNC_OUT
			}

			double compFirstLot() {
				MARK_FUNC_IN
				//Если включен манименеджмент
				if (inputAutoMM > 0) {
					curdil->base_lot = normLot(floor(equity / inputMMEquity) * symbolLotStep);
				}
				MARK_FUNC_OUT
				return min(curdil->base_lot, inputMaxLot);
			}

			bool compSignal() {
				MARK_FUNC_IN

				//Первый ордер открываем в любом случае
				if (inputFirstFree[curdil->type] && curdil->level == 0) {
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->level >= inputFreeLvl) {
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->type) { //Продажи
					if (curdil->step_peak) {
						if (
							(curdil->step_peak - curdil->mpo >= deltaRollback) ||
							(deltaRollback == 0)
						) {
							curdil->step_peak = 0;
							MARK_FUNC_OUT
							return true;
						}
						else {
							curdil->step_peak = max(curdil->step_peak, curdil->mpo);
						}

					}
					else if (curdil->mpo > indicator->up[0]) {
						curdil->step_peak = curdil->mpo;
					}
				}
				else { //Покупки
					if (curdil->step_peak) {
						if (
							(curdil->mpo - curdil->step_peak >= deltaRollback) ||
							(deltaRollback == 0)
						) {
							curdil->step_peak = 0;
							MARK_FUNC_OUT
							return true;
						}
						else {
							curdil->step_peak = min(curdil->step_peak, curdil->mpo);
						}
					}
					else if (curdil->mpo < indicator->down[0]) {
						curdil->step_peak = curdil->mpo;
					}
				}

				MARK_FUNC_OUT
				return false;
			}

		private:
			double	profits[50];
			fxc::indicator::RAIndicator* indicator;

			inline void openNextOrder() {
				MARK_FUNC_IN
				double openprice = curdil->mpo;

				//Расстояние до будущего ордера, если отрицательное, то проехали
				if (curdil->delta(curdil->sl(curdil->last->openprice, deltaStep), openprice) > 0) {
					//Пока не можем выставить не отложку не по рынку
					curdil->step_peak = 0;
					MARK_FUNC_OUT
					return;
				}

				double slprice		= curdil->sl(openprice, deltaSL);
				double tpprice		= curdil->tp(openprice, deltaTP);
				double nextProfit	= profits[curdil->level - 1];
				double lots = (nextProfit * curdil->base_lot - curdil->basketWeight(tpprice, inputAveragingLevel)) / deltaTP;

				lots = max(lots, curdil->base_lot);

				lots = min(lots, inputMaxLot);

				createOrder(curdil->type, lots, openprice, slprice, tpprice);
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