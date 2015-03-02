#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace strategy {

	class AbstractStrategy : public fxc::Simbiot {

		public:

			AbstractStrategy(char* _symbol) : Simbiot(_symbol) {}

			// возвращает количество операций на текущем тике
			virtual int getJob() = 0;

			// Инициализация цикла обновления ордеров, поскольку цикл получается рваным (каждая итерация вызывается из MQL),
			// нельзя пользоватья результатами в MQL программе до завершения цикла
			virtual void refresh_init(double _ask, double _bid, double _equity) {
				dillers[0]->mpo = dillers[1]->mpc = _ask;
				dillers[1]->mpo = dillers[0]->mpc = _bid;

				((OrdersManager*) this)->reset();
			}

			virtual void refresh_prices(double *closes, double *highs, double *lows, int bars) {
				updateChartData(closes, highs, lows, bars);
			}

		protected:



	};

}

}