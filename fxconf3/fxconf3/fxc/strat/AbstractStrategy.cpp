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
			bool tradeAllowed = false;		//флаг разрешения или запрета торговых операций
			std::string reason;				//причина запрета торговых операций

			AbstractStrategy() :
				TerminalInfo() 	{
			}
			virtual void initStrategy() {};   //Инициализация стратегии
			virtual inline const bool bypass() { return false; }  //Функция интелектуального пропуска тиков, для ускорения оптимизации
			virtual void Strategy() {};   // Тут, в наследнике, должен быть код стратегии

			void init() {
				MARK_FUNC_IN
				k_point = (symbolDigits == 3 || symbolDigits == 5) ? 10 : 1;
				deltaCalc();
				initOrdersManager(this);
				printRegisteredProps();
				initStrategy();
				MARK_FUNC_OUT
				
			}
			// возвращает количество операций на текущем тике
			int getJob() {
				MARK_FUNC_IN
				resetActionManager();
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

			//Проверка уровня стоплосса или тейкпрофита
			inline bool check_sl(int type, double low_price, double high_price) {
				return !(low_price && high_price && dillers[type % 2]->delta(low_price, high_price) < deltaStopLevel);
			}

			//Проверка на правильность параметров при открытии ордера
			inline int check_new(int type, double* lots, double* openprice, double* slprice, double* tpprice) {
				type = type % 2;
				if (
					type < 2 && // Для немедленного исполнения
					(
					!check_sl(type, *slprice, dillers[type]->mpc) ||
					!check_sl(type, dillers[type]->mpc, *tpprice)
					)
					) {
					return 201;
				}

				if (
					type > 1 &&  // Для отложенных ордеров
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