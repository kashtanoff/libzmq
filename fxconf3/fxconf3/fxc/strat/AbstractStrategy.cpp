#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace strategy {

	class AbstractStrategy : public fxc::Simbiot {

		public:

			AbstractStrategy(char* _symbol) : Simbiot(_symbol) {}

			// возвращает количество операций на текущем тике
			int getJob() {
				MARK_FUNC_IN
				((TradeManager*) this)->reset();
				sortOrders();

				for (int i = 0; i < 2; i++) {
					curdil = dillers[i];

					Strategy();   //Тут в наследнике должна быть вся торговая логика
				}
				MARK_FUNC_OUT
				return getActionsStackSize();
			}
			// Инициализация цикла обновления ордеров, поскольку цикл получается рваным (каждая итерация вызывается из MQL),
			// нельзя пользоватья результатами в MQL программе до завершения цикла
			virtual int bypass(double _ask, double _bid, double _equity) {
				dillers[0]->mpo = dillers[1]->mpc = _ask;
				dillers[1]->mpo = dillers[0]->mpc = _bid;

				((OrdersManager*) this)->reset();
			}

			virtual void refresh_prices(double *closes, double *highs, double *lows, int bars) {
				updateChartData(closes, highs, lows, bars);
			}

		protected:
			//Тут, в наследнике, должен быть код стратегии
			virtual void Strategy() {}
			double	equity;
			Diller* curdil;


	};

}

}