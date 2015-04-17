#pragma once

#include "../fxc.h"
#include "../debug/Debug.h"
#include "../ActionManager.cpp"
#include "../Diller.cpp"


#include "../ChartData.cpp"
#include <string>
#include <vector>

namespace fxc {

	class AbstractStrategy :  
		public ActionManager,
		public TimeSeries {

		public:

			int breakStatus = NO_BREAK;
			std::string status; // �������� ������ ������ ���������
			std::string reason; // ������� ������� �������� ��������

			AbstractStrategy() {
			}

			virtual void initStrategy() {};   //������������� ���������
			virtual inline const bool bypass() { return false; }  //������� ���������������� �������� �����, ��� ��������� �����������
			virtual void Strategy() {};   // ���, � ����������, ������ ���� ��� ���������
			virtual void showInfo() {};

			void init() {
				MARK_FUNC_IN
				k_point = (symbolDigits == 3 || symbolDigits == 5) ? 10 : 1;
				initOrdersManager();
				printRegisteredProps();
				initStrategy();
				//��������: ��� ������� =)
				status = "Trade Allowed";
				reason = "All OK";
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
				if (is_visual)
					showInfo();
				MARK_FUNC_OUT
				return getActionsStackSize();
			}

			// ������������� ����� ���������� �������, ��������� ���� ���������� ������ (������ �������� ���������� �� MQL),
			// ������ ����������� ������������ � MQL ��������� �� ���������� �����
			const bool tickInitBegin(double _ask, double _bid, double _equity, double _balance) {
				MARK_FUNC_IN
				ask		= _ask;
				bid		= _bid;
				equity	= _equity;
				balance = _balance;

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
				//msg << "newBars: " << (int)getChartData(3)->newBars << "\r\n" << msg_box;
				invokeListeners();
				resetOrderManager();
				MARK_FUNC_OUT
			}


		protected:
			
			Diller* curdil;
	};

}