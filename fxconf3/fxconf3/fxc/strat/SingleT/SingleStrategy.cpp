#pragma once

#include "Defines.h"
#include "../../fxc.h"
#include "../../Format.h"
#include "../../debug/Debug.h"
#include "../../indicators/RAIndicator.cpp"
#include "Parameters.cpp"
#include "../AbstractStrategy.cpp"

namespace fxc {

namespace strategy {

	class SingleStrategy : 
		public AbstractStrategy,
		public Parameters 
	{

		public:
			double wait_higher=0;
			double wait_lower=100000;
			bool block[2];
			double prevprice[2];
			ChartData*		  rates;

			SingleStrategy() :  
				AbstractStrategy(),
				Parameters((CPropertyList*) this)
			{
				
			}

			~SingleStrategy() {
				delete indicator;
			}

			virtual void initStrategy() {
				MARK_FUNC_IN

				paramsDeltaCalc(k_point * symbolPoint);
				for (int i = 0, l = sizeof(profits) / sizeof(*profits); i < l; i++) {
					profits[i] = deltaTP * pow(inputPipsMultiplier, i);
				}
				
				indicator = new fxc::ra_indicator::RAIndicator(this, inputTimeFrame, inputPeriod1, inputPeriod2, inputDeviation, deltaMinDev);
				rates = getChartData(inputTimeFrame); 
				dillers[OP_BUY]->base_lot = inputBaseLot[OP_BUY];
				dillers[OP_SELL]->base_lot	= inputBaseLot[OP_SELL];
				block[OP_BUY] = false;
				block[OP_SELL] = false;
				prevprice[OP_BUY] = 1000;
				prevprice[OP_SELL] = 0;
				
				if (inputSetName.find(symbolName) == std::string::npos) {
					setStatus(PROVIDER_STRATEGY, STATUS_HARD_BREAK, "wrong set name", "it has to contain " + symbolName);
				}
				MARK_FUNC_OUT
			}

			virtual void filterOrders() {
				if (!inputAverageAll) {
					AbstractStrategy::filterOrders();
				}
			}

		protected:
			inline virtual const bool bypass() {	
				/*if (wait_higher <= ask || wait_lower >= bid) {
					wait_higher = 100000;
					wait_lower = 0;
					return false;
				}
				return true;	*/
				return false;
			}
			inline void h(double high){
				wait_higher = min(wait_higher, high);
			}
			inline void l(double low){
				wait_lower = max(wait_lower, low);
			}
			virtual void Strategy() {
				MARK_FUNC_IN
				// ���� ��� ������� � �����
				if (!curdil->level) {  //���� ������� ���, �� ���� �����, ��������� ������
					//block[curdil->type] = false;
					if (breakStatus == STATUS_SOFT_BREAK) {
						breakStatus = STATUS_HARD_BREAK;  //����� ���������� ����������, �������� ������ ������
						status    = "trading stopped";
						MARK_FUNC_OUT
						return;
					}
					if (!inputStopNew[curdil->type] && compSignal() && curdil->delta(indicator->middle[0], indicator->middle[inputPowerPeriod]) < deltaMaxPower) {
/*#if DEBUG
						if (is_visual) {
							curdil->open_reason = "0-" + curdil->open_reason;
							msg << "open: " << curdil->type << " - " << curdil->open_reason << "\r\n" << msg_box;
							curdil->open_reason = "";
						}
#endif*/
						wait_higher = 0;
						createOrder(
							curdil->type,
							compFirstLot(),
							curdil->mpo,
							curdil->sl(curdil->mpo, deltaSL),
							curdil->tp(curdil->mpo, deltaFirstTP),
							"1-" + inputCommentText
						);
						//curdil->open_reason = "";
						if (inputCloseMode) {
							autoClose();
						}
					}
				}
				else {  //���� ���� ������ � �����
					moveTP();
					if (curdil->type) //�������
						h(curdil->last->openprice + deltaStep);  //���� ������������ ����
					else
						l(curdil->last->openprice - deltaStep);  //���� ������������ ����
					// ���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
					if (!inputStop[curdil->type] && curdil->level < inputMaxGridLevel) {
						if (compSignal()) {
							wait_higher = 0;
							openNextOrder();
							block[curdil->type] = true;
							if (inputCloseMode == 2) {
								autoClose();
							}
						}
					}
					// ���� ���� ������, �� �� ��������� ��������� -> ������� �������
					else {
						delStopLimitOrders();
					}
				}
				prevprice[curdil->type] = curdil->mpo;
				MARK_FUNC_OUT
			}
			virtual void onOrderClose(int ticket) {
				drawOrder(ticket);
			}

			virtual void showInfo() {
				MARK_FUNC_IN
				using namespace fxc::utils;
				/*
				AsciiTable table;
				table
					.setCell("BuyLevel:") .right().setCell(Format::decimal(dillers[0]->level,      0) + "   ").reserv(8).down() // ������� ����� �� �������
					.setCell("BuyLots:")  .right().setCell(Format::decimal(dillers[0]->total_lots, 2)).down()         // ��������� �������� �� �������
					.setCell("BuyDD:")    .right().setCell(Format::decimal(dillers[0]->open_dd,    2)).down()         // �������� �� �������
					.setCell("SellLevel:").right().setCell(Format::decimal(dillers[1]->level,      0) + "   ").down() // ������� ����� �� �������
					.setCell("SellLots:") .right().setCell(Format::decimal(dillers[1]->total_lots, 2)).down()         // ��������� �������� �� �������
					.setCell("SellDD:")   .right().setCell(Format::decimal(dillers[1]->open_dd,    2)).down()         // �������� �� �������
					.setCell("SymbolDD:") .right().setCell(Format::decimal(dillers[0]->open_dd 
					                                                     + dillers[1]->open_dd,    2)).down()        // ����� ��������
#if DEBUG
					.setCell("up:").right().setCell(Format::decimal(indicator->up[0], 5)).down()
					.setCell("down:").right().setCell(Format::decimal(indicator->down[0], 5)).down()
					.setCell("close:").right().setCell(Format::decimal(getChartData(inputTimeFrame)->close[0], 5)).down()
#endif
					//.setCell("PrevProfit:") .right().setCell("0").down()  // ������� �� ������� ������
					//.setCell("Profit:")     .right().setCell("0").down()  // ������� �� ������� ������
					//.setCell("O&C Balance:").right().setCell("0").down(); // ������ ������� � ���
					;
				std::stringstream ss(table.setAlign(1, AsciiTable::ALIGN_RIGHT).toString());
				std::string line;
				int i = 0;

				while (std::getline(ss, line, '\n')) {
					showValue(i++, line);
				}*/
				int i = 0;
				showValue(i++, Format::sformat("BuyLevel: %*d", 7, dillers[0]->level));
				showValue(i++, Format::sformat("BuyLots: %*.*f", 2, 10, dillers[0]->total_lots));
				showValue(i++, Format::sformat("BuyDD: %*d", 2, 10, dillers[0]->open_dd));
				showValue(i++, Format::sformat("SellLevel: %*d", 7, dillers[1]->level));
				showValue(i++, Format::sformat("SellLots: %*d", 2, 10, dillers[1]->total_lots));
				showValue(i++, Format::sformat("SellDD: %*d", 2, 10, dillers[1]->open_dd));
				showValue(i++, Format::sformat("SymbolDD: %*d", 2, 10, dillers[0]->open_dd + dillers[1]->open_dd));

				showValue(i++, status); // ������
				showValue(i++, reason); // �������
				MARK_FUNC_OUT
			}

			inline void moveTP() {
				MARK_FUNC_IN
				double tp = (curdil->level == 1) ? deltaFirstTP : deltaTP;
				double last_tpprice = curdil->tp(curdil->last->openprice, tp);
				double min_delta = max(symbolPoint, deltaStopLevel);
				if (curdil->type) //�������
					l(last_tpprice);  //���� �����������
				else  //�������
					h(last_tpprice); //���� �����������
				// ���� � ���������� ������ ��� �� ���������� ����������, �� ������ ���
				if (abs(curdil->last->tpprice - last_tpprice) > min_delta) {
					modOrder(
						curdil->last->ticket, 
						curdil->last->openprice, 
						curdil->sl(curdil->last->openprice, deltaSL),
						last_tpprice
					);
				}
				// ���� � ����� ���� �����, �� ������ � �������
				if (curdil->level < 2) {
					MARK_FUNC_OUT
					return;
				}

				double last_weigth  = curdil->orderWeight(curdil->last->openprice, last_tpprice, curdil->last->lots);
				double total_weight = curdil->basketWeight(last_tpprice);
				//msg << "lastweight: " << last_weigth << "\r\n";
				//msg << "total_weight: " << total_weight <<  "\r\n" << msg_box;

				// ���� ��� ���� ����� �����������, �� ����������� ��� ���������� ������ ��� ���������������� �������� ���� �����
				if (total_weight > 0) {
					last_weigth *= 10;
				}

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
			}

			double compFirstLot() {
				MARK_FUNC_IN
					//���� ������� ��������������
					if (inputAutoMM > 0) {
						curdil->base_lot = normLot(floor(balance / inputMMEquity) * symbolLotStep);
					}
				MARK_FUNC_OUT
					return min(curdil->base_lot, inputMaxLot);
			}
			bool compSignal() {
				MARK_FUNC_IN

					//������ ����� ��������� � ����� ������
					if (inputFirstFree[curdil->type] && curdil->level == 0) {
						//curdil->open_reason += "FirstFree";
						MARK_FUNC_OUT
							return true;
					}

				if (curdil->level >= inputFreeLvl) {
					//curdil->open_reason += "FreeLvl:" + fxc::utils::Format::decimal(inputFreeLvl, 0);
					MARK_FUNC_OUT
						return true;
				}
				if (curdil->type) { //�������
					//if (curdil->mpo < indicator->down[0] && prevprice[OP_SELL] >= indicator->down[0]) {
					if (curdil->mpo < rates->low[1] && prevprice[OP_SELL] >= rates->low[1] && indicator->middle[1] > indicator->middle[0]) {
						MARK_FUNC_OUT
						return true;
					}
				}
				else { //�������
					//if (curdil->mpo > indicator->up[0] && prevprice[OP_BUY] <= indicator->up[0]) {
					if (curdil->mpo > rates->high[1] && prevprice[OP_BUY] <= rates->high[1] && indicator->middle[1] < indicator->middle[0]) {
						MARK_FUNC_OUT
							return true;
					}
				}
				MARK_FUNC_OUT
					return false;
			}

			bool compSignalOld() {
				MARK_FUNC_IN

					//������ ����� ��������� � ����� ������
					if (inputFirstFree[curdil->type] && curdil->level == 0) {
						//curdil->open_reason += "FirstFree";
						MARK_FUNC_OUT
							return true;
					}

				if (curdil->level >= inputFreeLvl) {
					//curdil->open_reason += "FreeLvl:" + fxc::utils::Format::decimal(inputFreeLvl, 0);
					MARK_FUNC_OUT
						return true;
				}

				if (curdil->type) { //�������
					if (block[OP_SELL] && curdil->mpo < indicator->middle[0]) {
						block[OP_SELL] = false;
						//MARK_FUNC_OUT
						//return true;
					}
					if (curdil->trail_in_peak) {
						h(curdil->trail_in_peak);   //���� ������ ����
						l(curdil->trail_in_peak - deltaRollback);  //���� ������
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
					else if (curdil->mpo > indicator->up[0] && !block[OP_SELL]) {
						//curdil->open_reason += "i:" + fxc::utils::Format::decimal(indicator->up[0], 5) + "-";
						curdil->trail_in_peak = curdil->mpo;
						h(curdil->trail_in_peak);   //���� ������ ����
						l(curdil->trail_in_peak - deltaRollback);  //���� ������
					}
					else {
						h(indicator->up[0]);  //���� ������ ������
						h(getChartData(inputTimeFrame)->high[0]);  //���� ���������� ���������
					}
				}
				else { //�������
					if (block[OP_BUY] && curdil->mpo > indicator->middle[0]) {
						block[OP_BUY] = false;
						//MARK_FUNC_OUT
						//return true;
					}

					if (curdil->trail_in_peak) {
						l(curdil->trail_in_peak); //���� ������ ����
						h(curdil->trail_in_peak + deltaRollback);  //���� ������
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
					else if (curdil->mpo < indicator->down[0] && !block[OP_BUY]) {
						//curdil->open_reason += "i:"+fxc::utils::Format::decimal(indicator->down[0], 5) + "-";
						curdil->trail_in_peak = curdil->mpo;
						l(curdil->trail_in_peak); //���� ������ ����
						h(curdil->trail_in_peak + deltaRollback);  //���� ������
					}
					else {
						l(indicator->down[0]);  //���� ������ ������
						l(getChartData(inputTimeFrame)->low[0]);  //���� ���������� ��������
					}
				}
				MARK_FUNC_OUT
					return false;
			}

		private:

			double profits[50];
			fxc::ra_indicator::RAIndicator* indicator;

			inline void openNextOrder() {
				MARK_FUNC_IN
					using namespace fxc::utils;
				double openprice = curdil->mpo;

				//���������� �� �������� ������, ���� �������������, �� ��������
				if (curdil->delta(curdil->sl(curdil->last->openprice, deltaStep), openprice) > 0) {
					//���� �� ����� ��������� �� ������� �� �� �����
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
				/*
#if DEBUG
				if (is_visual) {
					curdil->open_reason = Format::decimal(curdil->level + 1, 0) + "-" + curdil->open_reason;
					msg << "open: " << curdil->type << " - " << curdil->open_reason << "\r\n" << msg_box;
					curdil->open_reason = "";
				}
#endif*/
				createOrder(curdil->type, lots, openprice, slprice, tpprice, Format::decimal(curdil->level+1, 0) + "-" + inputCommentText);
				//curdil->open_reason = "";
				MARK_FUNC_OUT
			}

			inline void autoClose() {
				MARK_FUNC_IN

				if (curdil->opposite->basketCost() > 0 || inputCloseMode == 3) {
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