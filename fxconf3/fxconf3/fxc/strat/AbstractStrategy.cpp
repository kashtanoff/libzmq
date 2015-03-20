#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace strategy {

	class AbstractStrategy : public fxc::Simbiot {

		public:

			AbstractStrategy(char* _symbol) : Simbiot(_symbol) {}

			// ���������� ���������� �������� �� ������� ����
			int getJob() {
				MARK_FUNC_IN
				((TradeManager*) this)->reset();
				sortOrders();

				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];

					Strategy();   //��� � ���������� ������ ���� ��� �������� ������
				}
				MARK_FUNC_OUT
				return getActionsStackSize();
			}
			// ������������� ����� ���������� �������, ��������� ���� ���������� ������ (������ �������� ���������� �� MQL),
			// ������ ����������� ������������ � MQL ��������� �� ���������� �����
			virtual int bypass(double _ask, double _bid, double _equity) {
				dillers[0]->mpo = dillers[1]->mpc = _ask;
				dillers[1]->mpo = dillers[0]->mpc = _bid;

				((OrdersManager*) this)->reset();
			}

			virtual void refresh_prices(double *closes, double *highs, double *lows, int bars) {
				updateChartData(closes, highs, lows, bars);
			}

		protected:
			//���, � ����������, ������ ���� ��� ���������
			virtual void Strategy() {}
			double	equity;
			Diller* curdil;


	};

}

}