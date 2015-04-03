#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace strategy {

	class AbstractStrategy : public fxc::Simbiot {

		public:

			AbstractStrategy(char* _symbol) : Simbiot(_symbol) {
			}

			// возвращает количество операций на текущем тике
			int getJob() {
				MARK_FUNC_IN

				((TradeManager*) this)->reset();
				sortOrders();

				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];
					Strategy(); // Тут в наследнике должна быть вся торговая логика
				}

				MARK_FUNC_OUT
				return getActionsStackSize();
			}

			// Инициализация цикла обновления ордеров, поскольку цикл получается рваным (каждая итерация вызывается из MQL),
			// нельзя пользоватья результатами в MQL программе до завершения цикла
			const bool tickInitBegin(double _ask, double _bid, double _equity) {
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
				getTimeseries()->reset();

				MARK_FUNC_OUT
				return false;
			}

			virtual inline const bool bypass() {
				return true;
			}

			virtual void tickInitEnd() {
				MARK_FUNC_IN
				
				getTimeseries()->updateFirst(ask, bid);
				getTimeseries()->invokeListeners();

				((OrdersManager*) this)->reset();
				MARK_FUNC_OUT
			}

		protected:

			// Тут, в наследнике, должен быть код стратегии
			virtual void Strategy() = 0;
			double	ask;
			double	bid;
			double	equity;
			Diller* curdil;

	};

}

}