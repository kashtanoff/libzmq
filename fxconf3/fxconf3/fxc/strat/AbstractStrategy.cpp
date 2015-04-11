#pragma once

#include "../OrdersManager.cpp"
#include "../TradeManager.cpp"
#include "../Diller.cpp"

#include "../TerminalInfo.cpp"
#include "../ChartData.cpp"
#include <string>
#include <vector>

namespace fxc {

namespace strategy {

	class AbstractStrategy : 
		public TerminalInfo,
		public OrdersManager,
		public TradeManager,
		public TimeSeries {

		public:
			bool tradeAllowed = false;		//���� ���������� ��� ������� �������� ��������
			std::string reason;				//������� ������� �������� ��������

			AbstractStrategy() :
				TerminalInfo() 	{
			}
			virtual void initStrategy() {};   //������������� ���������
			virtual inline const bool bypass() { return false; }  //������� ���������������� �������� �����, ��� ��������� �����������
			virtual void Strategy() {};   // ���, � ����������, ������ ���� ��� ���������

			void init() {
				MARK_FUNC_IN
				k_point = (symbolDigits == 3 || symbolDigits == 5) ? 10 : 1;
				deltaCalc();
				initOrdersManager(this);
				printRegisteredProps();
				initStrategy();
				MARK_FUNC_OUT
				
			}
			// ���������� ���������� �������� �� ������� ����
			int getJob() {
				MARK_FUNC_IN
				resetActionManager();
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
			const bool tickInitBegin(double _ask, double _bid, double _equity) {
				MARK_FUNC_IN
				MARK_FUNC_IN
				ask    = _ask;
				bid    = _bid;
				equity = _equity;

				dillers[0]->mpo = dillers[1]->mpc = ask;
				dillers[1]->mpo = dillers[0]->mpc = bid;
				if (bypass()) {
					MARK_FUNC_OUT
					return true;
				}
				timeSeriesReset();

				MARK_FUNC_OUT
				return false;
			}

			void tickInitEnd() {
				MARK_FUNC_IN
				updateFirst(ask, bid);
				invokeListeners();
				((OrdersManager*) this)->reset();
				MARK_FUNC_OUT
			}
			/*
			inline fxc::TimeSeries* const getTimeseries() {
				return timeseries;
			}

			inline fxc::ChartData* const getChartData(const int timeframe) {
				return timeseries->getChartData(timeframe);
				*/
#pragma region Checks

			//�������� ������ ��������� ��� �����������
			inline bool check_sl(int type, double low_price, double high_price) {
				return !(low_price && high_price && dillers[type % 2]->delta(low_price, high_price) < deltaStopLevel);
			}

			//�������� �� ������������ ���������� ��� �������� ������
			inline int check_new(int type, double* lots, double* openprice, double* slprice, double* tpprice) {
				type = type % 2;
				if (
					type < 2 && // ��� ������������ ����������
					(
					!check_sl(type, *slprice, dillers[type]->mpc) ||
					!check_sl(type, dillers[type]->mpc, *tpprice)
					)
					) {
					return 201;
				}

				if (
					type > 1 &&  // ��� ���������� �������
					(
					!check_sl(type, *openprice, *tpprice) ||
					!check_sl(type, *slprice, *openprice)
					)
					) {
					return 201;
				}

				if (
					(type == OP_BUYLIMIT || type == OP_SELLLIMIT) &&
					!check_sl(type, *openprice, dillers[type]->mpo)
					) {
					return 202;
				}

				if (
					(type == OP_BUYSTOP || type == OP_SELLSTOP) &&
					!check_sl(type, dillers[type]->mpo, *openprice)
					) {
					return 202;
				}

				*lots = normLot(*lots);
				*openprice = normPrice(*openprice);
				*tpprice = normPrice(*tpprice);
				*slprice = normPrice(*slprice);

				return 0;
			}
			inline double normLot(double value) {
				value = int(ceil(value / symbolLotStep)) * symbolLotStep;
				value = max(value, symbolMinLot);
				return min(value, symbolMaxLot);
			}

			inline double normPrice(double value) {
				return floor(value / symbolPoint + 0.5) * symbolPoint;
			}
		protected:
			double	ask;
			double	bid;
			double	equity;
			Diller* curdil;
	};
}

}