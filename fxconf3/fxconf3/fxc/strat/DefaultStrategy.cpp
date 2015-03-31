#pragma once

#include "AbstractStrategy.cpp"
#include "../debug/Debug.h"
#include "../indicators/RAIndicator.cpp"

namespace fxc {

namespace strategy {

	class DefaultStrategy : public AbstractStrategy {

		public:

			DefaultStrategy(char* _symbol) : AbstractStrategy(_symbol) {
			}

			virtual void init() {
				MARK_FUNC_IN
				Simbiot::init();
				indicator = new fxc::indicator::RAIndicator(this, input_timeframe, input_period, input_periodf2, input_deviation, input_delta);
				MARK_FUNC_OUT
			}

		protected:

			virtual void Strategy() {
				MARK_FUNC_IN

				// ���� ��� ������� � �����
				if (!curdil->level) {
					// ���� �� ��������� �������� ����� ����� � ���� ������
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

					// ���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
					if (!input_stop_avr[curdil->type] && curdil->level < input_max_grid_lvl) {

						// ���� ��������� �������� � ��� ����� ���������
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
					// ���� ���� ������, �� �� ��������� ��������� -> ������� �������
					else {
						delStopLimitOrders();
					}
				}
				MARK_FUNC_OUT
			}

			inline void moveTP() {
				MARK_FUNC_IN
				double last_tpprice = curdil->tp(curdil->last->openprice, input_takeprofit);

				// ���� � ���������� ������ ��� �� ���������� ����������, �� ������ ���
				if (abs(curdil->last->tpprice - last_tpprice) > input_point) {
					modOrder(
						curdil->last->ticket, 
						curdil->last->openprice, 
						curdil->sl(curdil->last->openprice, input_stoploss), 
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
				//��������������� ������� ������� ���������� �� ��������
				curdil->cur_av_lvl = input_av_lvl;
				
				double base_lot;

				//���� ������� ��������������
				if (input_auto_mm > 0) {
					base_lot = normLot(floor(equity / input_mm_equ) * input_lot_step);
					curdil->_base_lot = base_lot;
				}
				else {
					base_lot = curdil->_base_lot;
				}

				double result = base_lot;

				//������������ ���������� ��� ������
				if (curdil->opposite->level >= input_trend_lvl) {
					result = max(result, base_lot * input_trend_lot_mult * pow(input_trend_progress, curdil->opposite->level - input_trend_lvl));
				}
				//������������ ���������� ��� �������
				else if (curdil->prev_lvl >= input_repeat_lvl) {
					result = max(result, base_lot * input_repeat_lot_mult * pow(input_repeat_progress, curdil->prev_lvl - input_repeat_lvl));
				}

				//������������ ������� ������������
				if (curdil->opposite->last)
					result = max(result, curdil->opposite->last->lots * input_lot_hadge_mult);

				if (input_weighthadge)
					result = max(result, (input_takeprofit * base_lot - curdil->opposite->basketWeight(tpprice, 100) * input_weighthadge) / input_takeprofit);

				result = max(result, *(ext_total_lots + curdil->opposite->type) * input_basket_hadge_mult);
				result = max(result, curdil->prev_lots * input_regres_mult); //���������
				result = min(result, input_maxlot);

				MARK_FUNC_OUT
				return result;
			}

			bool compSignal() {
				MARK_FUNC_IN

				//������ ����� ��������� � ����� ������
				if (input_first_free && curdil->level == 0) {
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->level >= input_free_lvl) {
					MARK_FUNC_OUT
					return true;
				}

				if (curdil->type) { //�������
					if (curdil->step_peak) {
						if (
							(input_periodf3 && indicator->middle[0] < indicator->middle[1]) ||
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
					else if (curdil->mpo > indicator->up[0]) {
						curdil->step_peak = curdil->mpo;
					}
				}
				else { //�������
					if (curdil->step_peak) {
						if (
							(input_periodf3 && indicator->middle[0] < indicator->middle[1]) ||
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
					else if (curdil->mpo < indicator->down[0]) {
						curdil->step_peak = curdil->mpo;
					}
				}

				MARK_FUNC_OUT
				return false;
			}

		private:

			fxc::indicator::RAIndicator* indicator;

			inline void tryOpenForward() {
				MARK_FUNC_IN
				if (curdil->opposite->level >= input_forward_lvl && !curdil->ord_stop) {
					int    o_type;
					double openprice = curdil->tp(curdil->first->openprice, input_step * input_forward_step_mult);
					double d = curdil->delta(curdil->mpo, openprice);
					
					// ���� ����� ��������� �������
					if (d > input_min_sl_tp) {
						o_type = curdil->type + 4;
					}
					// ���� ������ �������, �� ����� �� �����
					else if (d <= 0) {
						openprice = curdil->mpo;
						o_type    = curdil->type;
					}
					// ���� �� ����� ��������� �� ������� �� �� �����
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

				//���������� �� �������� ������, ���� �������������, �� ��������
				if (curdil->delta(curdil->sl(curdil->last->openprice, input_step), openprice) > 0) {
					//���� �� ����� ��������� �� ������� �� �� �����
					curdil->step_peak = 0;
					MARK_FUNC_OUT
					return;
				}

				//����������� ���������� �� ������� ����
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