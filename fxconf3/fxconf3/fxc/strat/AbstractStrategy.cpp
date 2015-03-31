#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace strategy {

	class AbstractStrategy : public fxc::Simbiot {

		public:

			AbstractStrategy(char* _symbol) : Simbiot(_symbol) {
				MARK_FUNC_IN
				MARK_FUNC_OUT
			}

			// ���������� ���������� �������� �� ������� ����
			int getJob() {
				MARK_FUNC_IN

				((TradeManager*) this)->reset();
				sortOrders();

				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];
					Strategy(); // ��� � ���������� ������ ���� ��� �������� ������
				}

				MARK_FUNC_OUT
				return getActionsStackSize();
			}

			// ������������� ����� ���������� �������, ��������� ���� ���������� ������ (������ �������� ���������� �� MQL),
			// ������ ����������� ������������ � MQL ��������� �� ���������� �����
			virtual const bool tickInitBegin(double _ask, double _bid, double _equity) {
				ask    = _ask;
				bid    = _bid;
				equity = _equity;

				return true;
			}

			virtual void tickInitEnd() {
				getTimeseries()->reset();
				getTimeseries()->updateFirst(ask, bid);
				getTimeseries()->invokeListeners();

				dillers[0]->mpo = dillers[1]->mpc = ask;
				dillers[1]->mpo = dillers[0]->mpc = bid;

				((OrdersManager*) this)->reset();
			}

		protected:

			// ���, � ����������, ������ ���� ��� ���������
			virtual void Strategy() = 0;
			double	ask;
			double	bid;
			double	equity;
			Diller* curdil;

	};

}

}