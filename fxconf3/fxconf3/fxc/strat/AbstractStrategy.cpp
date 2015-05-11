#pragma once

#include "../fxc.h"
#include "../debug/Debug.h"
#include "../ActionManager.cpp"
#include "../Diller.cpp"


#include "../ChartData.cpp"
#include <string>
#include <vector>

namespace fxc {

	struct ErrorStatus {
		int statusType;
		std::string	status;
		std::string	reason;
	};

	class AbstractStrategy :
		public ActionManager,
		public TimeSeries {

	public:

		int breakStatus = STATUS_HARD_BREAK;
		std::string status; // Основной статус работы советника
		std::string reason; // причина запрета торговых операций
		double onTesterValue = 0;  //Оптимизационный критерий

		AbstractStrategy() {
		}

		virtual void initStrategy() {};   //Инициализация стратегии
		virtual inline const bool bypass() { return false; }  //Функция интелектуального пропуска тиков, для ускорения оптимизации
		virtual void Strategy() {};   // Тут, в наследнике, должен быть код стратегии
		virtual void showInfo() {};
		virtual void onChangeStatus(int _provider) {}
		//virtual bool checkErrors() { return false; }

		void init() {
			MARK_FUNC_IN
				for (int provider = 0; provider < PROVIDERS_COUNT; provider++) {
					statuses[provider].statusType = STATUS_OK;
					statuses[provider].status = "";
					statuses[provider].reason = "";
				}
			k_point = (symbolDigits == 3 || symbolDigits == 5) ? 10 : 1;
			initOrdersManager();
			printRegisteredProps();
			status = "trade is not allowed";
			reason = "cheking permissions";
			if (!mqlTradeAllowed) {
				msg << "autotrade not allowed\r\n" << msg_box;
				setStatus(PROVIDER_DLL, STATUS_DANGER, "auto-tade is not allowed", "press auto-trade button");
			}
			initStrategy();
			msg << "strategy init done\r\n" << msg_box;
			MARK_FUNC_OUT
		}

		// возвращает количество операций на текущем тике
		int getJob() {
			MARK_FUNC_IN
			resetActionManager();
			sortOrders();
			if (breakStatus != STATUS_HARD_BREAK) {
				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];
					Strategy(); // Тут в наследнике должна быть вся торговая логика
				}
			}
			if (is_visual)
				showInfo();
			MARK_FUNC_OUT
			return getActionsStackSize();
		}
		
		// Инициализация цикла обновления ордеров, поскольку цикл получается рваным (каждая итерация вызывается из MQL),
		// нельзя пользоватья результатами в MQL программе до завершения цикла
		const bool tickInitBegin(double _ask, double _bid, double _equity, double _balance) {
			MARK_FUNC_IN
			ask = _ask;
			bid = _bid;
			equity = _equity;
			balance = _balance;

			dillers[0]->mpo = dillers[1]->mpc = ask;
			dillers[1]->mpo = dillers[0]->mpc = bid;
			if (mqlOptimization && bypass()) {  //Только в режиме оптимизации пытаемся экономить на пропуске тиков
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

		void setStatus(int _provider, int _work_status, std::string _status, std::string _reason) {
			statuses[_provider].statusType = _work_status;
			statuses[_provider].status = _status;
			statuses[_provider].reason = _reason;
			breakStatus = _work_status;
			status = _status;
			reason = _reason;
			for (int provider = 0; provider < PROVIDERS_COUNT; provider++) {
				if (statuses[provider].statusType > _work_status) {
					status = statuses[provider].status;
					reason = statuses[provider].reason;
					breakStatus = statuses[provider].statusType;
				}
			}
			msg << "Status (" << breakStatus << "): " << status << ", " << reason << "\r\n" << msg_box;
			/*if (new_status != breakStatus)
				onChangeStatus(_provider);*/
		}
		virtual double getOnTester() {
			return 0;
		}

		protected:
			ErrorStatus statuses[PROVIDERS_COUNT];
			Diller* curdil;
	};

}