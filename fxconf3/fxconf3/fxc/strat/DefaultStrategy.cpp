#pragma once

#include "../fxc.h"
#include "../Format.h"
#include "../debug/Debug.h"
#include "../indicators/RAIndicator.cpp"
#include "../Parameters.cpp"
#include "AbstractStrategy.cpp"

namespace fxc {

namespace strategy {

	class DefaultStrategy : 
		public AbstractStrategy,
		public Parameters 
	{

		public:

			DefaultStrategy() :  
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
				indicator = new fxc::indicator::RAIndicator(this, inputTimeFrame, inputPeriod1, inputPeriod2, inputDeviation, deltaMinDev);
				dillers[OP_BUY]->base_lot	= inputBaseLot[OP_BUY];
				dillers[OP_SELL]->base_lot	= inputBaseLot[OP_SELL];
				MARK_FUNC_OUT
			}

		protected:
			//inline virtual const bool bypass() {	return true;	}
			virtual void Strategy() {
				MARK_FUNC_IN
					
				// ���� ��� ������� � �����
				if (!curdil->level) {  //���� ������� ���, �� ���� �����, ��������� ������
					if (breakStatus == SOFT_BREAK) {
						breakStatus = HARD_BREAK;  //����� ���������� ����������, �������� ������ ������
						status    = "Averaging done. Trading stopped.";
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

					// ���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
					if (!inputStop[curdil->type] && curdil->level < inputMaxGridLevel) {
						if (compSignal()) {
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
					.setCell("BuyLevel:") .right().setCell(Format::decimal(dillers[0]->level,      0)).down() // ������� ����� �� �������
					.setCell("BuyLots:")  .right().setCell(Format::decimal(dillers[0]->total_lots, 2)).down() // ��������� �������� �� �������
					.setCell("BuyDD:")    .right().setCell(Format::decimal(dillers[0]->open_dd,    2)).down() // �������� �� �������
					.setCell("SellLevel:").right().setCell(Format::decimal(dillers[1]->level,      0)).down() // ������� ����� �� �������
					.setCell("SellLots:") .right().setCell(Format::decimal(dillers[1]->total_lots, 2)).down() // ��������� �������� �� �������
					.setCell("SellDD:")   .right().setCell(Format::decimal(dillers[1]->open_dd,    2)).down() // �������� �� �������
					.setCell("SymbolDD:") .right().setCell(Format::decimal(dillers[0]->open_dd 
					                                                     + dillers[1]->open_dd,    2)).down() // ����� ��������
					.setCell("PrevProfit:") .right().setCell("0").down()  // ������� �� ������� ������
					.setCell("Profit:")     .right().setCell("0").down()  // ������� �� ������� ������
					.setCell("O&C Balance:").right().setCell("0").down(); // ������ ������� � ���

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
				double tp = (curdil->level == 1) ? deltaFirstTP : deltaTP;
				double last_tpprice = curdil->tp(curdil->last->openprice, tp);
				double min_delta = max(symbolPoint, deltaStopLevel);

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

				// ���� ��� ���� ����� �����������, �� ����������� ��� ���������� ������ ��� ���������������� �������� ���� �����
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
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->level >= inputFreeLvl) {
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->type) { //�������
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
				else { //�������
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

			double profits[50];
			fxc::indicator::RAIndicator* indicator;

			inline void openNextOrder() {
				MARK_FUNC_IN
				double openprice = curdil->mpo;

				//���������� �� �������� ������, ���� �������������, �� ��������
				if (curdil->delta(curdil->sl(curdil->last->openprice, deltaStep), openprice) > 0) {
					//���� �� ����� ��������� �� ������� �� �� �����
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