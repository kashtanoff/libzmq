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
		int oldTradeAllowed;
		std::string status;        // Основной статус работы советника
		std::string reason;        // причина запрета торговых операций
		double onTesterValue = 0;  // Оптимизационный критерий
		double mm_const = 0;

		AbstractStrategy() {}

		virtual void initStrategy() {};                       // Инициализация стратегии
		virtual inline const bool bypass() { return false; }  // Функция интелектуального пропуска тиков, для ускорения оптимизации
		virtual void Strategy() = 0;                          // Тут, в наследнике, должен быть код стратегии
		virtual void showInfo() {};
		virtual void onChangeStatus(int _provider) {}
		//virtual bool checkErrors() { return false; }

		virtual void filterOrders() {
			auto ptr    = getOrdersPtr();
			auto length = getOrdersLength();
			int  offset = 0;

			for (int i = 0; i < length; i++) {
				auto order = ptr + i;

				if (
					(order->magic == OBSOLETE_MAGIC_OC | ((expertMagic & MAGIC_MASK_EA) >> 8) | (expertMagic & MAGIC_MASK_USR)) ||
					((order->magic & MAGIC_MASK) != (expertMagic & MAGIC_MASK))
				) {
					offset++;
				}
				else if (offset > 0) {
					*(order - offset) = *order;
				}
			}

			if (offset > 0) {
				length -= offset;
			}
		}

		void init() {
			MARK_FUNC_IN
			for (int provider = 0; provider < PROVIDERS_COUNT; provider++) {
				statuses[provider].statusType = STATUS_OK;
				statuses[provider].status = "";
				statuses[provider].reason = "";
			}

			k_point = (symbolDigits == 3 || symbolDigits == 5) ? 10 : 1;
			mm_const = (STANDART_CONTRACT / symbolLotSize) * symbolLotStep;
			
			initOrdersManager();
			//printRegisteredProps();
			
			status = "trade is not allowed";
			reason = "cheking permissions";
			initStrategy();
			msg << "strategy init done\r\n" << msg_box;
			MARK_FUNC_OUT
		}

		// возвращает количество операций на текущем тике
		int getJob() {
			MARK_FUNC_IN
			resetActionManager();
			filterOrders();
			fillDillers();
			sortOrders();
			

			if (breakStatus != STATUS_HARD_BREAK) {
				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];
					if (breakStatus < STATUS_SOFT_BREAK || curdil->level) 
					{
						Strategy(); // Тут в наследнике должна быть вся торговая логика
					}
				}
			}
			if (is_visual) {
				if (mqlTradeAllowed != oldTradeAllowed) {
					oldTradeAllowed = mqlTradeAllowed;
					fxc::mutex.lock();
					if (mqlTradeAllowed) {
						setStatus(PROVIDER_TERMINAL, STATUS_OK, "auto-trade allowed", "all ok");
					}
					else {
						setStatus(PROVIDER_TERMINAL, STATUS_DANGER, "auto-trade not allowed", "press auto-trade button");
					}
					fxc::mutex.unlock();
				}
				showInfo();
			}
			MARK_FUNC_OUT
			return getActionsStackSize();
		}
		
		// Инициализация цикла обновления ордеров, поскольку цикл получается рваным (каждая итерация вызывается из MQL),
		// нельзя пользоватья результатами в MQL программе до завершения цикла
		const bool tickInitBegin(double _ask, double _bid, double _equity, double _balance) {
			MARK_FUNC_IN
			ask = _ask;
			bid = _bid;
			equity  = _equity;
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
			//msg << "Status (" << breakStatus << "): " << status << ", " << reason << "\r\n" << msg_box;
			/*if (new_status != breakStatus)
				onChangeStatus(_provider);*/
		}
		virtual double getOnTester() {
			return 0;
		}
		//Закрывает все ордера для указанного диллера
		inline void closeAll(Diller* dil) {
			for (int i = 0; i < dil->level; i++) {
				closeOrder(
					dil->orders[i]->ticket,
					dil->orders[i]->lots,
					dil->mpc
					);
			}
		}
		//Закрывает все ордера текущего диллера
		inline void closeAll() {
			closeAll(curdil);
		}
		//Расчитывает лот для разных режимов манименеджмента
		inline double calcMMLot(int MMType, double MMDD, double minLot = 0.01) {
			switch (MMType){
			case MM_NOMM:
				return normLot(minLot);
			case MM_BALANCE:
				return normLot(floor(balance / MMDD) * mm_const);
			case MM_EQUITY:
				return normLot(floor(equity / MMDD) * mm_const);
			}
			return symbolMinLot;
		}
		protected:
			ErrorStatus statuses[PROVIDERS_COUNT];
			Diller* curdil;
	};

}