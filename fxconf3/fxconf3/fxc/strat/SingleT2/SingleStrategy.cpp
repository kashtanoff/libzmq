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
		class StepChannel :
			public CascadRule
		{
			virtual bool rule() {

			}
		};

		class SingleStrategy :
			public AbstractStrategy,
			public Parameters
		{

		public:
			double wait_higher = 0;
			double wait_lower = 100000;
			bool block[2];
			double prevprice[2];
			ChartData*		  rates;
			double baseProfit;
			double profit[2];  //эксп неваляшка

			SingleStrategy() :
				AbstractStrategy(),
				Parameters((CPropertyList*) this)
			{

			}

			virtual void initStrategy() {
				MARK_FUNC_IN

					paramsDeltaCalc(k_point * symbolPoint);
				for (int i = 0, l = sizeof(profits) / sizeof(*profits); i < l; i++) {
					profits[i] = deltaTP * pow(inputPipsMultiplier, i);
				}
				baseProfit = deltaTP * 0.01;

				channel = new fxc::ra_indicator::RAIndicator(this, inputTimeFrame, inputPeriod1, inputPeriod2, inputDeviation, deltaMinDev);
				rates = getChartData(inputTimeFrame);
				fastma[0] = new fxc::iLWMA(this, inputFastTimeFrame, inputFastPeriod, PRICE_TYPICAL);
				fastma[1] = new fxc::iLWMA(this, inputFastTimeFrame, inputFastPeriod, PRICE_TYPICAL);
				//slowma = new fxc::iLWMA(this, 30, 100, PRICE_TYPICAL);
				
				for (int op = OP_BUY; op <= OP_SELL; op++) {
					dillers[op]->base_lot = inputBaseLot[op];
					//dillers[op]->trail_in_init(deltaRollback);
					block[op] = false;
					//dillers[op]->c_rule = new CascadRule([&]()->bool {return step(dillers[op]) && price_channel(dillers[op]); },
					//	new CascadRule([&]()->bool {return fastma_speed(op) > 0; }));
					profit[op] = 0;   //эксп неваляшка
				}
				if (inputSetName.find(symbolName) == std::string::npos) {
					setStatus(PROVIDER_STRATEGY, STATUS_HARD_BREAK, "wrong set name", "it has to contain " + symbolName);
				}
				MARK_FUNC_OUT
			}

			~SingleStrategy() {
				delete channel;
				delete fastma[0];
				delete fastma[1];
			}

		protected:
			inline virtual const bool bypass() {
				/*if (ask >= wait_higher || bid <= wait_lower) {
				wait_higher = 100000;
				wait_lower = 0;
				return false;
				}
				return true;	*/
				return false;
			}
			inline void wh(double high){
				wait_higher = min(wait_higher, high);
			}
			inline void wl(double low){
				wait_lower = max(wait_lower, low);
			}
			//Основной торговый код
			virtual void Strategy() {
				MARK_FUNC_IN
					// Если нет ордеров в рынке
					if (!curdil->level) {  //Если ордеров нет
						//profit[curdil->type] = 0;
						if (!inputStopNew[curdil->type] && compSignal()) {//Если не запрещено открывать первый и есть сигнал
							curdil->ban_bar = rates->time[0];
							curdil->opposite->ban_bar = 0;
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
						if (!inputStop[curdil->type] && curdil->level < inputMaxGridLevel && compSignal()) {
							curdil->ban_bar = rates->time[0];
							curdil->opposite->ban_bar = 0;

							openNextOrder();
							
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

				AsciiTable table;
				table
					.setCell("BuyLevel:").right().setCell(Format::decimal(dillers[0]->level, 0) + "   ").reserv(8).down() // Уровень сетки на покупку
					.setCell("BuyLots:").right().setCell(Format::decimal(dillers[0]->total_lots, 2)).down()         // Суммарная лотность на покупку
					.setCell("BuyDD:").right().setCell(Format::decimal(dillers[0]->open_dd, 2)).down()         // Просадка на покупку
					.setCell("SellLevel:").right().setCell(Format::decimal(dillers[1]->level, 0) + "   ").down() // Уровень сетки на продажу
					.setCell("SellLots:").right().setCell(Format::decimal(dillers[1]->total_lots, 2)).down()         // Суммарная лотность на продажу
					.setCell("SellDD:").right().setCell(Format::decimal(dillers[1]->open_dd, 2)).down()         // Просадка на продажу
					.setCell("SymbolDD:").right().setCell(Format::decimal(dillers[0]->open_dd
					+ dillers[1]->open_dd, 2)).down()        // Общая просадка
#if DEBUG
					.setCell("up:").right().setCell(Format::decimal(channel->up[0], 5)).down()
					.setCell("down:").right().setCell(Format::decimal(channel->down[0], 5)).down()
					//.setCell("ma:").right().setCell(Format::decimal(fastma->ma[0], 5)).down()
					//.setCell("speed:").right().setCell(Format::decimal(abs(fastma->ma[1] - fastma->ma[0]), 5)).down()
					.setCell("close:").right().setCell(Format::decimal(rates->close[0], 5)).down()
					.setCell("deltaTP:").right().setCell(Format::decimal(deltaTP, 5)).down()
					//.setCell("price | channel:").right().setCell(Format::decimal((int)price_channel(), 0) + "   ").down()
					//.setCell("rollback:").right().setCell(Format::decimal((int)rollback(), 0) + "   ").down()
					.setCell("buy mode:").right().setCell(Format::decimal((int)dillers[0]->trail_in_mode, 0) + "   ").down()
					.setCell("sell mode:").right().setCell(Format::decimal((int)dillers[1]->trail_in_mode, 0) + "   ").down()
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
				}

				showValue(i++, status); // Статус
				showValue(i++, reason); // Причина
				MARK_FUNC_OUT
			}

			inline void moveTP() {
				MARK_FUNC_IN
					double tp = (curdil->level == 1) ? deltaFirstTP : deltaTP;
					double last_tpprice = curdil->tp(curdil->last->openprice, tp);
					double min_delta = max(symbolPoint, deltaStopLevel);
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
					double last_weigth = curdil->orderWeight(curdil->last->openprice, last_tpprice, curdil->last->lots);
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
				profit[curdil->type] = 0;
				MARK_FUNC_OUT
					return min(curdil->base_lot, inputMaxLot);
			}

			bool compSignal() {
				MARK_FUNC_IN
				//Первый ордер открываем в любом случае
				if (firstFree() || freeLevel()) {
					MARK_FUNC_OUT
					return true;
				}
				/*
				if (tostrong()) {
					curdil->trail_in_mode = 0;
					closeAll();
					return false;
				}*/
				//if (curdil->level == 0 && yes_candle()) return true;
				switch (curdil->trail_in_mode){
					//Трейлинг не включен
					case 0: if (step() && price_channel()){// && ban_bar() && !fastma_block()) {
								if (fastma_channel() || !ban_bar() || no_candle()) {
									//curdil->trail_in_mode = 1;   //Ма в канал
									/*if (curdil->level > 0) {
										closeAll(curdil);
										openOppositeOrder();
									}*/
								}
								//else if (!normal_speed()) {
								//	curdil->trail_in_mode = 3;
								//}
								else {
									curdil->trail_in_mode = 2;   //Откат
									curdil->trail_in_peak = curdil->mpo;
								}
							}
							break;
					//Отрабатываем быструю машку
					case 1: if (!fastma_channel()) {
								if (inputRallyBlockMode) {
									curdil->trail_in_mode = 4;
								}
								else {
									curdil->trail_in_mode = 0;
								}
								return true;
							}
							break;
					//Отрабатываем откат
					case 2: if (curdil->check_peak(deltaRollback)) {
								if (inputRallyBlockMode) {
									curdil->trail_in_mode = 4;
								}
								else {
									curdil->trail_in_mode = 0;
								}
								return true;
							}
							break;
					//case 3: return fastma_speed()>0.0;
					case 4: if (price_middle()) {
						curdil->trail_in_mode = 0;
					}
				}
				/*if (curdil->trail_in_peak > 0) {
					MARK_FUNC_OUT
					return curdil->trail_in_stop();
					}
				else {
					if (step() && price_channel() && !fastma_channel()){// && no_candle()) {
						curdil->trail_in_start();
					}
				}*/
				MARK_FUNC_OUT
				return false;
			}
#pragma region Сигналы и фильтры
			//Разрешает открытие первого ордера без доп условий
			inline bool firstFree() {
				return inputFirstFree[curdil->type] && curdil->level == 0;
			}
			//Разрешает свободное оппозитное открытие
			inline bool freeLevel() {
				return curdil->opposite->level >= inputFreeLvl;
			}
			//Пропускает минимальный шаг от последнего ордера
			inline bool step() {
				if (curdil->level == 0) {
					return true;
				}
				return (curdil-> type) ?
					curdil->mpo - curdil->last->openprice > deltaStep:
					curdil->last->openprice - curdil->mpo > deltaStep;
			}
			//цена прбила границы канала
			inline bool price_channel() {
				return (curdil->type) ? curdil->mpo > channel->up[0]: curdil->mpo < channel->down[0];
			}
			//Быстрая машка пробила канал
			inline bool fastma_channel() {
				if (curdil->level == 0) { return false; }
				return (curdil->type) ?
					fastma[1]->ma[0] > channel->up[0] : //Продажи
					fastma[0]->ma[0] < channel->down[0]; //Покупки
			}
			inline bool fastma_block() {
				return (inputRallyBlockMode == 2 && fastma_channel());
			}
			//Пересечение быстрой машкой границы канала
			inline bool fastma_x_channel() {
				return (curdil->type) ?
					fastma[1]->ma[1] > channel->up[1] && fastma[1]->ma[0] < channel->up[0] :  //Продажи
				fastma[0]->ma[1] < channel->down[1] && fastma[0]->ma[0] > channel->down[0];  //Покупки
			}
			//Есть откат
			inline bool rollback(){
				return (curdil->type) ?
					rates->high[0] - deltaRollback >= curdil->mpo || rates->high[1] - deltaRollback >= curdil->mpo :
					rates->low[0] + deltaRollback <= curdil->mpo || rates->low[1] + deltaRollback >= curdil->mpo;
			}
			//Скорость меньше критической
			inline bool normal_speed() {
				return abs(fastma[curdil->type]->ma[1] - fastma[curdil->type]->ma[0]) < inputFastSpeed;
			}

			inline double fastma_speed() {
				return (curdil->type) ?
					fastma[1]->ma[1] - fastma[1]->ma[0] :
					fastma[0]->ma[0] - fastma[0]->ma[1];
			}
			//Пробойная свеча сигнал к открытию
			inline bool yes_candle() {
				return (curdil->type)?
					rates->high[0] > channel->middle[0] && curdil->mpo < channel->down[0]: //Продажи
					rates->low[0] < channel->middle[0] && curdil->mpo > channel->up[0]; //Покупки
			}
			//Пробойная свеча отсутствует
			inline bool no_candle() {
				return (curdil->type) ?
					rates->low[0] < channel->middle[0] :
					rates->high[0] > channel->middle[0];
			}
			//Если сработал трейлинг
			inline bool trail_in_stop() {
				return true;
			}

			inline bool step_price_channel() {
				return step() && price_channel();
			}
			inline bool price_middle() {
				return (curdil->type) ?
					curdil->mpo < channel->middle[0] :
					curdil->mpo > channel->middle[0];
			}
			inline bool ban_bar() {
				return curdil->ban_bar != rates->time[0];
			}
			inline bool trend1() {
				if (curdil->level>0) return true;
				return (curdil->type) ?
					(channel->up[0] - channel->middle[0]) < (channel->middle[0] - channel->down[0]) :
					(channel->up[0] - channel->middle[0]) > (channel->middle[0] - channel->down[0]);
			}
			inline bool trend2() {
				if (curdil->level>0) return true;
				return (curdil->type)?
					slowma->ma[1] > slowma->ma[0]:
					slowma->ma[1] < slowma->ma[0];
			}
			inline bool trend3() {
				if (curdil->level>0) return true;
				return (curdil->type) ?
					slowma->ma[1] > slowma->ma[0] && curdil->mpo < slowma->ma[0]:
				slowma->ma[1] < slowma->ma[0] && curdil->mpo > slowma->ma[0];
			}
			inline bool tostrong() {
				double strong = (curdil->type) ?
					(channel->up[0] - channel->middle[0]) / (channel->middle[0] - channel->down[0]):
					(channel->middle[0] - channel->down[0]) / (channel->up[0] - channel->middle[0]);
				return strong > (double)inputMaxPower;

			}
#pragma endregion

		private:

			double profits[50];
			fxc::ra_indicator::RAIndicator* channel;
			fxc::iLWMA* fastma[2];
			fxc::iLWMA* slowma;

			inline void openNextOrder() {
				MARK_FUNC_IN
					using namespace fxc::utils;
				double openprice = curdil->mpo;
				double slprice = curdil->sl(openprice, deltaSL);
				if (inputFastSpeed > 0) {
					deltaTP = (curdil->type) ? channel->middle[0] - channel->down[0] : channel->up[0] - channel->middle[0];
					if (deltaTP < inputFastSpeed) return;
				}
				double tpprice = curdil->tp(openprice, deltaTP);
				double nextProfit = profits[curdil->level - 1];
				double lots = (nextProfit * curdil->base_lot - curdil->basketWeight(tpprice, inputAveragingLevel)) / deltaTP;

				lots = max(lots, curdil->base_lot);
				lots = min(lots, inputMaxLot);
				createOrder(curdil->type, lots, openprice, slprice, tpprice, Format::decimal(curdil->level + 1, 0) + "-" + inputCommentText);
				MARK_FUNC_OUT
			}
			inline void openOppositeOrder() {
				MARK_FUNC_IN
					using namespace fxc::utils;
				double openprice = curdil->opposite->mpo;
				double slprice = curdil->opposite->sl(openprice, deltaSL);
				double tpprice = curdil->opposite->tp(openprice, deltaTP);
				double nextProfit = profits[curdil->opposite->level - 1];
				double sum_profit = profit[0] + profit[1];
				double lots = (nextProfit * curdil->opposite->base_lot + sum_profit - curdil->basketWeight(curdil->mpc, inputAveragingLevel)) / deltaTP;
				profit[curdil->opposite->type] = sum_profit - curdil->basketWeight(curdil->mpc, inputAveragingLevel);
				profit[curdil->type] = 0;

				lots = max(lots, curdil->opposite->base_lot);
				lots = min(lots, inputMaxLot);
				createOrder(curdil->opposite->type, lots, openprice, slprice, tpprice, Format::decimal(curdil->opposite->level + 1, 0) + "-op-" + inputCommentText);
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
			inline void closeAll(Diller* dil) {
				for (int i = 0; i < dil->level; i++) {
					closeOrder(
						dil->orders[i]->ticket,
						dil->orders[i]->lots,
						dil->mpc
						);
				}
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