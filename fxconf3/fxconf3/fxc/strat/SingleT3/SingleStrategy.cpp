#pragma once

#include "Defines.h"
#include "../../fxc.h"
#include "../../Format.h"
#include "../../debug/Debug.h"
#include "../../indicators/RAIndicator.cpp"
#include "../../indicators/iLWMA.cpp"
#include "../../indicators/iWPR.cpp"
#include "Parameters.cpp"
#include "../AbstractStrategy.cpp"

namespace fxc {

	namespace strategy {

		class SingleStrategy :
			public AbstractStrategy,
			public Parameters
		{

		public:
			double closedDD[2];   //�������� �������� ��� ���������
			double prevprice[2];
			ChartData*		  rates;
			double baseProfit;
			double profit[2];  //���� ���������
			double profits[50];
			ra_indicator::RAIndicator* channel;
			iLWMA* fastma[3];
			iLWMA* slowma;
			iWPR* wpr;


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
				//fastma[2] = new fxc::iLWMA(this, inputFastTimeFrame, inputFastPeriod, PRICE_TYPICAL);
				slowma = new fxc::iLWMA(this, inputTimeFrame, inputPowerPeriod, PRICE_TYPICAL);
				wpr = new iWPR(this, inputTimeFrame, inputPowerPeriod);
				
				for (int op = OP_BUY; op <= OP_SELL; op++) {
					dillers[op]->base_lot = inputBaseLot[op];
					//dillers[op]->trail_in_init(deltaRollback);
					//dillers[op]->c_rule = new CascadRule([&]()->bool {return step(dillers[op]) && price_channel(dillers[op]); },
					//	new CascadRule([&]()->bool {return fastma_speed(op) > 0; }));
					profit[op] = 0;   //���� ���������
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
				//delete fastma[2];
				delete slowma;
				delete wpr;
			}

		protected:
			//�������� �������� ���
			virtual void Strategy() {
				MARK_FUNC_IN
					// ���� ��� ������� � �����
					if (!curdil->level) {  //���� ������� ���
						//profit[curdil->type] = 0;
						if (!inputStopNew[curdil->type] && compSignal()) {//���� �� ��������� ��������� ������ � ���� ������
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
					else {  //���� ���� ������ � �����
						moveTP();
						// ���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
						if (!inputStop[curdil->type] && curdil->level < inputMaxGridLevel && compSignal() && step()) {
							curdil->ban_bar = rates->time[0];
							curdil->opposite->ban_bar = 0;
							openNextOrder();
							autoClose();
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
				int i = 0;
				showValue(i++, Format::sformat("BuyLevel:  %*d", 9, dillers[0]->level));
				showValue(i++, Format::sformat("BuyLots:   %*.*f", 12, 2, dillers[0]->total_lots));
				showValue(i++, Format::sformat("BuyDD:     %*.*f", 12, 2, dillers[0]->open_dd));
				showValue(i++, Format::sformat("SellLevel: %*d", 9, dillers[1]->level));
				showValue(i++, Format::sformat("SellLots:  %*.*f", 12, 2, dillers[1]->total_lots));
				showValue(i++, Format::sformat("SellDD:    %*.*f", 12, 2, dillers[1]->open_dd));
				showValue(i++, Format::sformat("SymbolDD:  %*.*f", 12, 2, dillers[0]->open_dd + dillers[1]->open_dd));
#if DEBUG
				showValue(i++, Format::sformat("iWPR: %*.*f", 12, 5, wpr->wpr[0]));
#endif
				showValue(i++, status); // ������
				showValue(i++, reason); // �������
				MARK_FUNC_OUT
			}

			inline void moveTP() {
				MARK_FUNC_IN
					double tp = (curdil->level == 1) ? deltaFirstTP : deltaTP;
					double last_tpprice = curdil->tp(curdil->last->openprice, tp);
					double min_delta = max(symbolPoint, deltaStopLevel);
					
					//if (curdil->opposite->level == 1 && curdil->opposite->first->slprice == curdil->first->tpprice) {

					//}
					// ���� � ���������� ������ ��� �� ���������� ����������, �� ������ ���
					if (abs(curdil->last->tpprice - last_tpprice) > min_delta) {
						modOrder(
							curdil->last->ticket,
							curdil->last->openprice,
							curdil->sl(curdil->last->openprice, deltaSL),
							last_tpprice
							);
					}
					//������������ ���������� ����������
					if (curdil->opposite->level == 1) {
						double op_tp = curdil->opposite->first->tpprice;
						double op_sl = curdil->sl(op_tp, abs(curdil->mpc - curdil->mpo));
						if (curdil->opposite->orderWeight(curdil->opposite->first, op_tp) +
							curdil->orderWeight(curdil->first, op_sl) < 0) {
							op_sl = 0;
						}
						else if (abs(curdil->first->slprice - op_sl) > min_delta) {
							modOrder(
								curdil->first->ticket,
								curdil->first->openprice,
								op_sl,
								curdil->first->tpprice
								);
							//msg << "try move SL: wo=" << curdil->opposite->orderWeight(curdil->opposite->first, op_tp) << "\r\n" <<
							//	" wc=" << curdil->orderWeight(curdil->first, op_sl) << "\r\n" <<
							//	" p:" << op_sl << "\r\n" << msg_box;
						}
					}
					// ���� � ����� ���� �����, �� ������ � �������
					if (curdil->level < 2) {
						MARK_FUNC_OUT
							return;
					}
				MARK_FUNC_IN
					double last_weigth = curdil->orderWeight(curdil->last->openprice, last_tpprice, curdil->last->lots);
				double total_weight = curdil->basketWeight(last_tpprice);
				//msg << "lastweight: " << last_weigth << "\r\n";
				//msg << "total_weight: " << total_weight <<  "\r\n" << msg_box;

				// ���� ��� ���� ����� �����������, �� ����������� ��� ���������� ������ ��� ���������������� �������� ���� �����
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
					//���� ������� ��������������
					if (inputAutoMM > 0) {
						curdil->base_lot = normLot(floor(equity / inputMMEquity) * symbolLotStep);
					}
				/*
				profit[curdil->type] = 0;
				if (curdil->opposite->level > 0) {
					double tpprice = curdil->tp(curdil->mpo, deltaTP);
					moveSL(tpprice);
					MARK_FUNC_OUT
					return  (profits[0] * curdil->base_lot - curdil->opposite->basketWeight(tpprice, 1)) / deltaTP;
				}*/
				if (opstep(deltaStep * inputFastSpeed) && trend3()) {
					double tpprice = curdil->tp(curdil->mpo, deltaFirstTP);
					double spread = abs(curdil->mpo - curdil->mpc);
					double lots = (deltaFirstTP * curdil->base_lot - 
						curdil->opposite->orderWeight(curdil->opposite->first, curdil->opposite->sl(tpprice, spread))) / deltaFirstTP;
					if (lots < inputMaxLot / 100.0) {
						return lots;
					}
				}
				MARK_FUNC_OUT
					return min(curdil->base_lot, inputMaxLot);
			}

			bool compSignal() {
				MARK_FUNC_IN
				//������ ����� ��������� � ����� ������
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
					//�������� �� �������
					case 0: if (step() && price_channel() && ban_bar() && !fastma_block()) {
								/*if (!ban_bar() || no_candle()) {
									if (fastma_channel()) {
										curdil->trail_in_mode = 1;   //�� � �����
									}
									
									if (curdil->level > 1 && curdil->opposite->level == 0 &&
										trend3op() && (no_candle() || abs(rates->high[0]-rates->low[0]) > 2.0*deltaTP)) {
										//closeAll(curdil);
										openOppositeOrder();
									}
								}
								//else if (!normal_speed()) {
								//	curdil->trail_in_mode = 3;
								//}
								else {*/
						//if (curdil->level == 0 && !trend3()) return false;
									curdil->trail_in_mode = 2;   //�����
									curdil->trail_in_peak = curdil->mpo;
								//}
							}
							if (curdil->level == 0 && curdil->opposite->level >0 && trend3() && (price_middle() || opstep(deltaStep * inputFastSpeed))) {

								curdil->trail_in_mode = 2;   //�����
								curdil->trail_in_peak = curdil->mpo;
							}

							//else if (curdil->level == 0 && )
							break;
					//������������ ������� �����
					case 1: if (fastma_channel2() || !fastma_channel()) {
								if (inputRallyBlockMode) {
									curdil->trail_in_mode = 4;
								}
								else {
									curdil->trail_in_mode = 0;
								}
								return true;
							}
							break;
					//������������ �����
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
					case 4: if (!price_middle()) {
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
#pragma region ������� � �������
			//��������� �������� ������� ������ ��� ��� �������
			inline bool firstFree() {
				return inputFirstFree[curdil->type] && curdil->level == 0;
			}
			//��������� ��������� ���������� ��������
			inline bool freeLevel() {
				return curdil->opposite->level >= inputFreeLvl;
			}
			//���������� ����������� ��� �� ���������� ������
			inline bool step() {
				if (curdil->level == 0) {
					return true;
				}
				return (curdil->type) ?
					curdil->mpo - curdil->last->openprice > deltaStep:
					curdil->last->openprice - curdil->mpo > deltaStep;
			}
			inline bool opstep(double d) {
				if (curdil->opposite->level == 0) {
					return false;
				}
				return (curdil->opposite->type) ?
					curdil->mpo - curdil->opposite->first->openprice > d:
				curdil->opposite->first->openprice - curdil->mpo > d;
			}
			//���� ������ ������� ������
			inline bool price_channel() {
				return (curdil->type) ? curdil->mpo > channel->up[0]: curdil->mpo < channel->down[0];
			}
			//������� ����� ������� �����
			inline bool fastma_channel() {
				if (curdil->level == 0) { return false; }
				return (curdil->type) ?
					fastma[1]->ma[0] > channel->up[0] : //�������
					fastma[0]->ma[0] < channel->down[0]; //�������
			}
			inline bool fastma_channel2() {
				return (curdil->type) ?
					fastma[1]->ma[0] - fastma[1]->ma[1] < channel->up[0] - channel->up[1] && curdil->mpo > fastma[1]->ma[0] : //�������
					fastma[0]->ma[1] - fastma[0]->ma[0] < channel->up[1] - channel->up[0] && curdil->mpo < fastma[0]->ma[0]; //�������
			}
			inline bool fastma_block() {
				return (inputRallyBlockMode > 3 && fastma_channel());
			}
			//����������� ������� ������ ������� ������
			inline bool fastma_x_channel() {
				return (curdil->type) ?
					fastma[1]->ma[1] > channel->up[1] && fastma[1]->ma[0] < channel->up[0] :  //�������
				fastma[0]->ma[1] < channel->down[1] && fastma[0]->ma[0] > channel->down[0];  //�������
			}
			//���� �����
			inline bool rollback(){
				return (curdil->type) ?
					rates->high[0] - deltaRollback >= curdil->mpo || rates->high[1] - deltaRollback >= curdil->mpo :
					rates->low[0] + deltaRollback <= curdil->mpo || rates->low[1] + deltaRollback >= curdil->mpo;
			}
			//�������� ������ �����������
			inline bool normal_speed() {
				return abs(fastma[curdil->type]->ma[1] - fastma[curdil->type]->ma[0]) < inputFastSpeed;
			}

			inline double fastma_speed() {
				return (curdil->type) ?
					fastma[1]->ma[1] - fastma[1]->ma[0] :
					fastma[0]->ma[0] - fastma[0]->ma[1];
			}
			//��������� ����� ������ � ��������
			inline bool yes_candle() {
				return (curdil->type)?
					rates->high[0] > channel->middle[0] && curdil->mpo < channel->down[0]: //�������
					rates->low[0] < channel->middle[0] && curdil->mpo > channel->up[0]; //�������
			}
			//��������� ����� �����������
			inline bool no_candle() {
				return (curdil->type) ?
					rates->low[0] < channel->middle[0] :
					rates->high[0] > channel->middle[0];
			}
			//���� �������� ��������
			inline bool trail_in_stop() {
				return true;
			}

			inline bool step_price_channel() {
				return step() && price_channel();
			}
			inline bool price_middle() {
				return (curdil->type) ?
					curdil->mpo > channel->middle[0] :
					curdil->mpo < channel->middle[0];
			}
			inline bool ban_bar() {
				return inputRallyBlockMode < 3 || curdil->ban_bar != rates->time[0];
			}
			inline bool trend1() {
				if (curdil->level>0) return true;
				return (curdil->type) ?
					(channel->up[0] - channel->middle[0]) < (channel->middle[0] - channel->down[0]) :
					(channel->up[0] - channel->middle[0]) > (channel->middle[0] - channel->down[0]);
			}
			inline bool trend2() {
				//if (curdil->level>0) return true;
				return (curdil->type)?
					slowma->ma[1] > slowma->ma[0]:
					slowma->ma[1] < slowma->ma[0];
			}
			inline bool trend3op() {
				//if (curdil->level>0) return true;
				return (curdil->type) ?
					slowma->ma[1] < slowma->ma[0] && curdil->mpo > slowma->ma[0]:
				slowma->ma[1] > slowma->ma[0] && curdil->mpo < slowma->ma[0];
			}
			inline bool trend3() {
				//if (curdil->level>0) return true;
				return (curdil->type) ?
					slowma->ma[1] > slowma->ma[0]:// && curdil->mpo < slowma->ma[0] :
				slowma->ma[1] < slowma->ma[0];// && curdil->mpo > slowma->ma[0];
			}

			inline bool tostrong() {
				double strong = (curdil->type) ?
					(channel->up[0] - channel->middle[0]) / (channel->middle[0] - channel->down[0]):
					(channel->middle[0] - channel->down[0]) / (channel->up[0] - channel->middle[0]);
				return strong > (double)inputMaxPower;

			}
			inline bool tobig() {
				return abs(rates->high[0] - rates->low[0]) > 2.0*deltaTP;
			}
#pragma endregion

		private:


			inline void openNextOrder() {
				MARK_FUNC_IN
					using namespace fxc::utils;
				double spread = abs(curdil->mpo - curdil->mpc);
				double openprice = curdil->mpo;
				double slprice = curdil->sl(openprice, deltaSL);
				double tpprice = curdil->tp(openprice, deltaTP);
				double nextProfit = profits[curdil->level - 1];
				//double opposite = curdil->opposite->basketWeight(tpprice, inputAveragingLevel);
				double lots = (nextProfit * curdil->base_lot - curdil->basketWeight(tpprice, inputAveragingLevel)) / deltaTP;

				lots = max(lots, curdil->base_lot);
				lots = min(lots, inputMaxLot);
				createOrder(curdil->type, lots, openprice, slprice, tpprice, Format::decimal(curdil->level + 1, 0) + "-" + inputCommentText);
				
				if (curdil->opposite->level == 1 && curdil->opposite->first->slprice == curdil->last->tpprice) {
					modOrder(curdil->opposite->first->ticket,
						curdil->opposite->first->openprice,
						curdil->tp(tpprice, spread),
						curdil->opposite->first->tpprice
						);
				}
				//moveSL(tpprice);
				MARK_FUNC_OUT
			}
			inline void openOppositeOrder() {
				MARK_FUNC_IN
					using namespace fxc::utils;
				double spread = abs(curdil->mpo - curdil->mpc);
				double openprice = curdil->opposite->mpo;
				double slprice = curdil->opposite->sl(openprice, deltaSL);
				double tpprice = curdil->opposite->tp(openprice, deltaTP);
				double nextProfit = profits[0];
				//double sum_profit = profit[0] + profit[1];
				double lots = (nextProfit * curdil->opposite->base_lot - curdil->orderWeight(curdil->first, curdil->sl(tpprice, spread))) / deltaTP;
				//profit[curdil->opposite->type] = sum_profit - curdil->basketWeight(curdil->mpc, inputAveragingLevel);
				//profit[curdil->type] = 0;
				msg << "opposite: w=" << curdil->opposite->orderWeight(openprice, tpprice, lots) << " p:" << tpprice << "\r\n" << msg_box;
				msg << "covered   w=" << curdil->orderWeight(curdil->first, curdil->sl(tpprice, spread)) << " p:" << curdil->sl(tpprice, spread) << "\r\n" << msg_box;

				lots = max(lots, curdil->opposite->base_lot);
				lots = min(lots, inputMaxLot);
				if (lots > inputMaxLot / 100) return; 
				createOrder(curdil->opposite->type, lots, openprice, slprice, tpprice, Format::decimal(curdil->opposite->level + 1, 0) + "-op-" + inputCommentText);
				/*modOrder(curdil->first->ticket,
					curdil->first->openprice,
					curdil->sl(tpprice, spread),
					curdil->first->tpprice
					);*/
				MARK_FUNC_OUT
			}
			void moveSL(double sl) {

				for (int i = 0; i < curdil->opposite->level - 1; i++) {
						modOrder(
							curdil->opposite->orders[i]->ticket,
							curdil->opposite->orders[i]->openprice,
							sl,
							curdil->opposite->orders[i]->tpprice
							);
				}
			}

			inline void autoClose() {
				MARK_FUNC_IN
					bool flag = false;
					switch (inputCloseMode) {
						case 0: break; //�� ��������� ���������
						case 1: if (curdil->opposite->level == 1 && curdil->opposite->basketCost() > 0) {//��������� ���� ���� �����
									flag = true;
								}
								break;
						case 2: if (curdil->opposite->basketCost() > 0) {//��������� ���� ����� � ����
									flag = true;
								}
								break;
						case 3: flag = true; //��������� ����� ���� � ������
					}
					if (flag) {
						closeAll(curdil->opposite);
					}
				MARK_FUNC_OUT
			}
		};

	}

}