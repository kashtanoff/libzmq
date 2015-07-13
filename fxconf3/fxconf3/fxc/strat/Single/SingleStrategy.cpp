#pragma once

#include "Defines.h"
#include "../../fxc.h"
#include "../../Format.h"
#include "../../debug/Debug.h"
#include "../../indicators/RAIndicator.cpp"
#include "../../indicators/iLWMA.cpp"
#include "Parameters.cpp"
#include "../AbstractStrategy.cpp"

namespace fxc {

namespace strategy {

	class SingleStrategy : 
		public AbstractStrategy,
		public Parameters 
	{
		public:
			bool block[2];

			SingleStrategy() :  
				AbstractStrategy(),
				Parameters((CPropertyList*) this)
			{
				
			}

			virtual void initStrategy() {
				MARK_FUNC_IN
					if (!paramsCheck((CPropertyList*) this)) {
						setStatus(PROVIDER_STRATEGY, STATUS_EMERGENCY_BREAK, params_err1, params_err2);
					}
				paramsDeltaCalc(k_point * symbolPoint);
				for (int i = 0, l = sizeof(profits) / sizeof(*profits); i < l; i++) {
					profits[i] = deltaTP * pow(inputPipsMultiplier, i);
				}


				indicator = new fxc::ra_indicator::RAIndicator(this, inputTimeFrame, inputPeriod1, inputPeriod2, inputDeviation, deltaMinDev);
				if (inputRallyBlockMode > 2) {
					fastma = new fxc::iLWMA(this, inputTimeFrame, inputRallyBlockMode, PRICE_TYPICAL);
				}
				dillers[OP_BUY]->base_lot	= inputBaseLot[OP_BUY];
				dillers[OP_SELL]->base_lot	= inputBaseLot[OP_SELL];
				block[OP_BUY] = false;
				block[OP_SELL] = false;

				if (inputSetName.find(symbolName) == std::string::npos) {
					setStatus(PROVIDER_STRATEGY, STATUS_HARD_BREAK, "wrong set name", "it has to contain " + symbolName);
				}
				MARK_FUNC_OUT
			}

			~SingleStrategy() {
				delete indicator;
				delete fastma;
			}

		protected:
			virtual void Strategy() {
				MARK_FUNC_IN
				// Если нет ордеров в рынке
				if (!curdil->level) {  //Если ордеров нет, то если можно, открываем первый
					block[curdil->type] = false;
					if (breakStatus >= STATUS_SOFT_BREAK) {
						MARK_FUNC_OUT
							return;
					}

					if (!inputStopNew[curdil->type] && compSignal()) {
						createOrder(
							curdil->type,
							compFirstLot(),
							curdil->mpo,
							curdil->sl(curdil->mpo, deltaSL),
							curdil->tp(curdil->mpo, deltaFirstTP),
							"1-" + inputCommentText
						);
						autoClose();
					}
				}
				else {  //Если есть ордера в рынке
					moveTP();
					// Если есть базовый ордер и разрешено усреднять и максимальный уровень не достигнут
					if (!inputStop[curdil->type] && curdil->level < inputMaxGridLevel) {
						if (compSignal()) {
							openNextOrder();
							if ((inputRallyBlockMode == 1 && curdil->level == 1) || inputRallyBlockMode == 2) {
								block[curdil->type] = true;
							}
							autoClose();
						}
					}
				}
				MARK_FUNC_OUT
			}
			virtual void onOrderClose(int ticket) {
				drawOrder(ticket);
			}

			virtual void showInfo() {
				MARK_FUNC_IN
				using namespace fxc::utils;

				/*AsciiTable table;
				table
					.setCell("BuyLevel:") .right().setCell(Format::decimal(dillers[0]->level,      0) + "   ").reserv(8).down() // Уровень сетки на покупку
					.setCell("BuyLots:")  .right().setCell(Format::decimal(dillers[0]->total_lots, 2)).down()         // Суммарная лотность на покупку
					.setCell("BuyDD:")    .right().setCell(Format::decimal(dillers[0]->open_dd,    2)).down()         // Просадка на покупку
					.setCell("SellLevel:").right().setCell(Format::decimal(dillers[1]->level,      0) + "   ").down() // Уровень сетки на продажу
					.setCell("SellLots:") .right().setCell(Format::decimal(dillers[1]->total_lots, 2)).down()         // Суммарная лотность на продажу
					.setCell("SellDD:")   .right().setCell(Format::decimal(dillers[1]->open_dd,    2)).down()         // Просадка на продажу
					.setCell("SymbolDD:") .right().setCell(Format::decimal(dillers[0]->open_dd
					                                                     + dillers[1]->open_dd,    2)).down()        // Общая просадка
#if DEBUG
					.setCell("up:").right().setCell(Format::decimal(indicator->up[0], 5)).down()
					.setCell("down:").right().setCell(Format::decimal(indicator->down[0], 5)).down()
					//.setCell("ma:").right().setCell(Format::decimal(fastma->ma[0], 5)).down()
					//.setCell("speed:").right().setCell(Format::decimal(abs(fastma->ma[1] - fastma->ma[0]), 5)).down()
					//.setCell("close:").right().setCell(Format::decimal(getChartData(inputTimeFrame)->close[0], 5)).down()
#endif
					//.setCell("PrevProfit:") .right().setCell("0").down()  // Прибыль за прошлый период
					//.setCell("Profit:")     .right().setCell("0").down()  // Прибыль за текущий период
					//.setCell("O&C Balance:").right().setCell("0").down(); // Баланс средств у нас
					;
				std::stringstream ss(table.setAlign(1, AsciiTable::ALIGN_RIGHT).toString());
				std::string line;
				int i = 0;

				while (std::getline(ss, line, '\n')) {
					showValue(i++, line);
				}*/
				int i = 0;
				showValue(i++, Format::sformat("BuyLevel:  %*d",    9,    dillers[0]->level));
				showValue(i++, Format::sformat("BuyLots:   %*.*f", 12, 2, dillers[0]->total_lots));
				showValue(i++, Format::sformat("BuyDD:     %*.*f", 12, 2, dillers[0]->open_dd));
				showValue(i++, Format::sformat("SellLevel: %*d",    9,    dillers[1]->level));
				showValue(i++, Format::sformat("SellLots:  %*.*f", 12, 2, dillers[1]->total_lots));
				showValue(i++, Format::sformat("SellDD:    %*.*f", 12, 2, dillers[1]->open_dd));
				showValue(i++, Format::sformat("SymbolDD:  %*.*f", 12, 2, dillers[0]->open_dd + dillers[1]->open_dd));


				showValue(i++, status); // Статус
				showValue(i++, reason); // Причина
				MARK_FUNC_OUT
			}

			inline void moveTP() {
				MARK_FUNC_IN
				double tp = (curdil->level == 1) ? deltaFirstTP : deltaTP;
				MARK_FUNC_IN
				double last_tpprice = curdil->tp(curdil->last->openprice, tp);
				MARK_FUNC_IN
				double min_delta = max(symbolPoint, deltaStopLevel);
					MARK_FUNC_OUT
					MARK_FUNC_OUT
				// Если у последнего ордера еще не установлен тейкпрофит, то ставим его
				MARK_FUNC_IN
				if (abs(curdil->last->tpprice - last_tpprice) > min_delta) {
					modOrder(
						curdil->last->ticket, 
						curdil->last->openprice, 
						curdil->sl(curdil->last->openprice, deltaSL),
						last_tpprice
					);
				}
				MARK_FUNC_OUT
				// Если в рынке один ордер, то нечего и двигать
				if (curdil->level < 2) {
					MARK_FUNC_OUT
					return;
				}
				MARK_FUNC_IN
				double last_weigth  = curdil->orderWeight(curdil->last->openprice, last_tpprice, curdil->last->lots);
				double total_weight = curdil->basketWeight(last_tpprice);
				//msg << "lastweight: " << last_weigth << "\r\n";
				//msg << "total_weight: " << total_weight <<  "\r\n" << msg_box;

				// Если вес всей сетки положителен, то удесятеряем вес последнего ордера для гарантированного закрытия всей сетки
				if (total_weight > 0) {
					last_weigth *= 10;
				}
				MARK_FUNC_OUT
					MARK_FUNC_IN
				for (int i = 0; i < curdil->level - 1; i++) {
					//msg << "order[" << i << "] = " << curdil->orderWeight(curdil->orders[i]->openprice, last_tpprice, curdil->orders[i]->lots) <<  "\r\n" << msg_box;
					last_weigth += curdil->orderWeight(curdil->orders[i]->openprice, last_tpprice, curdil->orders[i]->lots);
					//msg << "lastweight: " << last_weigth << "\r\n" << msg_box;
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
					//curdil->open_reason += "FirstFree";
					MARK_FUNC_OUT
					return true;
				}
				if (curdil->level == 0 && curdil->opposite->level >= inputFreeLvl) {
					//curdil->open_reason += "FreeLvl:" + fxc::utils::Format::decimal(inputFreeLvl, 0);
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->type) { //Продажи
					if (block[OP_SELL] && curdil->mpo < indicator->middle[0]) {  //Пытаемся снять раллиблок
						block[OP_SELL] = false;
					}
					if (curdil->trail_in_peak) {
						if (
							(curdil->trail_in_peak - curdil->mpo >= deltaRollback) ||
							(deltaRollback == 0)
						) {
							//curdil->open_reason += "p:" + fxc::utils::Format::decimal(curdil->trail_in_peak, 5);
							curdil->trail_in_peak = 0;
							MARK_FUNC_OUT
							return true;
						}
						else {
							curdil->trail_in_peak = max(curdil->trail_in_peak, curdil->mpo);
						}

					}
					else if (curdil->mpo > indicator->up[0] && !block[OP_SELL] && 
						(inputRallyBlockMode<3 || fastma->ma[0] < indicator->up[0])) {
						//curdil->open_reason += "i:" + fxc::utils::Format::decimal(indicator->up[0], 5) + "-";
						curdil->trail_in_peak = curdil->mpo;
					}
				}
				else { //Покупки
					if (block[OP_BUY] && curdil->mpo > indicator->middle[0]) {  //Пытаемся снять раллиблок
						block[OP_BUY] = false;
					}
					if (curdil->trail_in_peak) {
						if (
							(curdil->mpo - curdil->trail_in_peak >= deltaRollback) ||
							(deltaRollback == 0)
							) {
							//curdil->open_reason += "p:" + fxc::utils::Format::decimal(curdil->trail_in_peak, 5);
							curdil->trail_in_peak = 0;
							MARK_FUNC_OUT
								return true;
						}
						else {
							curdil->trail_in_peak = min(curdil->trail_in_peak, curdil->mpo);
						}
					}
					else if (curdil->mpo < indicator->down[0] && !block[OP_BUY] &&
						(inputRallyBlockMode<3 || fastma->ma[0] > indicator->down[0])) {
						//curdil->open_reason += "i:"+fxc::utils::Format::decimal(indicator->down[0], 5) + "-";
						curdil->trail_in_peak = curdil->mpo;
					}
				}
				MARK_FUNC_OUT
				return false;
			}

		private:

			double profits[50];
			fxc::ra_indicator::RAIndicator* indicator;
			fxc::iLWMA* fastma;

			inline void openNextOrder() {
				MARK_FUNC_IN
					using namespace fxc::utils;
				double openprice = curdil->mpo;

				//Расстояние до будущего ордера, если отрицательное, то проехали
				if (curdil->delta(curdil->sl(curdil->last->openprice, deltaStep), openprice) > 0) {
					//Пока не можем выставить не отложку не по рынку
					curdil->trail_in_peak = 0;
					MARK_FUNC_OUT
					return;
				}

				double slprice		= curdil->sl(openprice, deltaSL);
				double tpprice		= curdil->tp(openprice, deltaTP);
				double nextProfit	= profits[curdil->level - 1];
				double lots = (nextProfit * curdil->base_lot - curdil->basketWeight(tpprice, inputAveragingLevel)) / deltaTP;

				lots = max(lots, curdil->base_lot);

				lots = min(lots, inputMaxLot);
				createOrder(curdil->type, lots, openprice, slprice, tpprice, Format::decimal(curdil->level+1, 0) + "-" + inputCommentText);
				//curdil->open_reason = "";
				MARK_FUNC_OUT
			}

			inline void autoClose() {
				MARK_FUNC_IN
					bool flag = false;
				switch (inputCloseMode) {
				case 0: break; //Не закрывать оппозитно
				case 1: if (curdil->opposite->level == 1 && curdil->opposite->basketCost() > 0) {//Закрывать если один ордер
					flag = true;
				}
						break;
				case 2: if (curdil->opposite->basketCost() > 0) {//Закрывать если сетка в плюс
					flag = true;
				}
						break;
				case 3: flag = true; //Закрывать сетку даже в убыток
				}
				if (flag) {
					closeAll(curdil->opposite);
				}
				MARK_FUNC_OUT
			}

	};

}

}