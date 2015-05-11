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

	class TriplexStrategy :
		public AbstractStrategy,
		public Parameters 
	{

		public:
			double wait_higher=0;
			double wait_lower=100000;
			int channel = 0;
			int totalGrid = 0;
			int lvlGrid = 0;

			TriplexStrategy() :
				AbstractStrategy(),
				Parameters((CPropertyList*) this)
			{
				
			}

			virtual void initStrategy() {
				MARK_FUNC_IN

				paramsDeltaCalc(k_point * symbolPoint);
				for (int i = 0, l = sizeof(profits) / sizeof(*profits); i < l; i++) {
					profits[i] = deltaTP[0] * pow(inputPipsMultiplier, i);
				}
				for (int i = 0; i < 3; i++) {
					indicator[i] = new fxc::ra_indicator::RAIndicator(this, inputTimeframe[i], inputPeriod[i], inputDevPeriod[i], inputDeviation[i], deltaMinDev);
				}
				dillers[OP_BUY]->base_lot = inputBaseLot[OP_BUY];
				dillers[OP_SELL]->base_lot	= inputBaseLot[OP_SELL];
				
				if (inputSetName.find(symbolName) == std::string::npos) {
					setStatus(PROVIDER_STRATEGY, STATUS_HARD_BREAK, "wrong set name", "it has to contain " + symbolName);
				}
				MARK_FUNC_OUT
			}

			~TriplexStrategy() {
				delete indicator[0];
				delete indicator[1];
				delete indicator[2];
			}
			virtual double getOnTester() {
				if (inputTestMode)
					return lvlGrid / totalGrid * 100;
				return 0;
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
					channel = compChannel();
				// ���� ��� ������� � �����
				if (!curdil->level) {  //���� ������� ���, �� ���� �����, ��������� ������
					if (curdil->prev_lvl) { //������ ��������� �����
						totalGrid++;
						if (curdil->prev_lvl == inputTestMode)  {//������� �������� �����
							lvlGrid++;
						}
						curdil->prev_lvl = 0;
					}
					if (breakStatus == STATUS_SOFT_BREAK) {
						breakStatus = STATUS_HARD_BREAK;  //����� ���������� ����������, �������� ������ ������
						status    = "trading stopped";
						MARK_FUNC_OUT
						return;
					}

					if (!inputStopNew[curdil->type] && compSignal()) {
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
							curdil->tp(curdil->mpo, deltaTP[0])
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
						h(curdil->last->openprice + deltaStep[channel]);  //���� ������������ ����
					else
						l(curdil->last->openprice - deltaStep[channel]);  //���� ������������ ����
					// ���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
					if (!inputStop[curdil->type] && curdil->level < inputMaxLevel[2]) {
						if (compSignal()) {
							wait_higher = 0;
							openNextOrder();
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
					.setCell("BuyLevel:") .right().setCell(Format::decimal(dillers[0]->level,      0) + "   ").reserv(8).down() // ������� ����� �� �������
					.setCell("BuyLots:")  .right().setCell(Format::decimal(dillers[0]->total_lots, 2)).down()         // ��������� �������� �� �������
					.setCell("BuyDD:")    .right().setCell(Format::decimal(dillers[0]->open_dd,    2)).down()         // �������� �� �������
					.setCell("SellLevel:").right().setCell(Format::decimal(dillers[1]->level,      0) + "   ").down() // ������� ����� �� �������
					.setCell("SellLots:") .right().setCell(Format::decimal(dillers[1]->total_lots, 2)).down()         // ��������� �������� �� �������
					.setCell("SellDD:")   .right().setCell(Format::decimal(dillers[1]->open_dd,    2)).down()         // �������� �� �������
					.setCell("SymbolDD:") .right().setCell(Format::decimal(dillers[0]->open_dd 
					                                                     + dillers[1]->open_dd,    2)).down()        // ����� ��������
#if DEBUG
					.setCell("channel:").right().setCell(Format::decimal(channel, 0)).down()
					.setCell("up:").right().setCell(Format::decimal(indicator[channel]->up[0], 5)).down()
					.setCell("down:").right().setCell(Format::decimal(indicator[channel]->down[0], 5)).down()
					.setCell("close:").right().setCell(Format::decimal(getChartData(inputTimeframe[channel])->close[0], 5)).down()
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
				}

				showValue(i++, status); // ������
				showValue(i++, reason); // �������
				MARK_FUNC_OUT
			}

			inline void moveTP() {
				MARK_FUNC_IN
				double tp = deltaTP[channel];
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
			int compChannel() {
				for (int i = 0; i < 3; i++) {
					if (curdil->level <= inputMaxLevel[i]) {
						return i;
					}
				}
				return 2;
			}
			double compFirstLot() {
				MARK_FUNC_IN
				//���� ������� ��������������
				if (inputAutoMM > 0) {
					curdil->base_lot = normLot(floor(equity / inputMMEquity) * symbolLotStep);
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
					if (curdil->step_peak) {
						h(curdil->step_peak);   //���� ������ ����
						l(curdil->step_peak - deltaRollback);  //���� ������
						if (
							(curdil->step_peak - curdil->mpo >= deltaRollback) ||
							(deltaRollback == 0)
						) {
							//curdil->open_reason += "p:" + fxc::utils::Format::decimal(curdil->step_peak, 5);
							curdil->step_peak = 0;
							MARK_FUNC_OUT
							return true;
						}
						else {
							curdil->step_peak = max(curdil->step_peak, curdil->mpo);
						}

					}
					else if (curdil->mpo > indicator[channel]->up[0]) {
						//curdil->open_reason += "i:" + fxc::utils::Format::decimal(indicator->up[0], 5) + "-";
						curdil->step_peak = curdil->mpo;
						h(curdil->step_peak);   //���� ������ ����
						l(curdil->step_peak - deltaRollback);  //���� ������
					}
					else {
						h(indicator[channel]->up[0]);  //���� ������ ������
						h(getChartData(inputTimeframe[channel])->high[0]);  //���� ���������� ���������
					}
				}
				else { //�������
					if (curdil->step_peak) {
						l(curdil->step_peak); //���� ������ ����
						h(curdil->step_peak + deltaRollback);  //���� ������
						if (
							(curdil->mpo - curdil->step_peak >= deltaRollback) ||
							(deltaRollback == 0)
							) {
							//curdil->open_reason += "p:" + fxc::utils::Format::decimal(curdil->step_peak, 5);
							curdil->step_peak = 0;
							MARK_FUNC_OUT
								return true;
						}
						else {
							curdil->step_peak = min(curdil->step_peak, curdil->mpo);
						}
					}
					else if (curdil->mpo < indicator[channel]->down[0]) {
						//curdil->open_reason += "i:"+fxc::utils::Format::decimal(indicator->down[0], 5) + "-";
						curdil->step_peak = curdil->mpo;
						l(curdil->step_peak); //���� ������ ����
						h(curdil->step_peak + deltaRollback);  //���� ������
					}
					else {
						l(indicator[channel]->down[0]);  //���� ������ ������
						l(getChartData(inputTimeframe[channel])->low[0]);  //���� ���������� ��������
					}
				}
				MARK_FUNC_OUT
				return false;
			}

		private:

			double profits[50];
			fxc::ra_indicator::RAIndicator* indicator[3];

			inline void openNextOrder() {
				MARK_FUNC_IN
					using namespace fxc::utils;
				double openprice = curdil->mpo;

				//���������� �� �������� ������, ���� �������������, �� ��������
				if (curdil->delta(curdil->sl(curdil->last->openprice, deltaStep[channel]), openprice) > 0) {
					//���� �� ����� ��������� �� ������� �� �� �����
					curdil->step_peak = 0;
					MARK_FUNC_OUT
					return;
				}

				double slprice		= curdil->sl(openprice, deltaSL);
				double tpprice = curdil->tp(openprice, deltaTP[channel]);
				double nextProfit	= profits[curdil->level - 1];
				double lots = (nextProfit * curdil->base_lot - curdil->basketWeight(tpprice, inputAveragingLevel)) / deltaTP[channel];

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
				createOrder(curdil->type, lots, openprice, slprice, tpprice);
				//curdil->open_reason = "";
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