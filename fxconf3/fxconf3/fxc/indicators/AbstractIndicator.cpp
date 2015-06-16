#pragma once

#include "../strat/AbstractStrategy.cpp" 

namespace fxc {

	class AbstractIndicator :
		ChartListener 
	{
		
		public:

			AbstractIndicator();
			AbstractIndicator(
				AbstractStrategy* manager,
				int timeframe,
				int outBufferLength
			) :
				manager(manager),
				timeframe(timeframe),
				outBufferLength(outBufferLength)
			{
				manager->registerTimeframe(timeframe, outBufferLength, this);
				rates = manager->getChartData(timeframe);
			}

			// Автоматически вызываемый метод, вызывается каждый тик перед запуском стратегии
			virtual inline void listenChart() {
				MARK_FUNC_IN
					int newBars = (rates->newBars > outBufferLength) ? outBufferLength : rates->newBars;
				//if (rates->newBars > outBufferLength) {
				//	msg << "listenChart to mach bars!!!\r\n" << msg_box;
				//}
				if (newBars) {
					for (auto& buffer : buffers) {
						buffer->skip(newBars);
					}
				}
				compute(newBars);
				MARK_FUNC_OUT
			}

			// Заготовка под метод вычисления индикатора
			virtual void compute(int newBars) = 0;
		
			void regBuffer(utils::CircularBuffer<double>* buffer) {
				buffer->alloc(outBufferLength);
				buffers.push_back(buffer);
			}
			std::function<double(int)> getPriceMethod(int price_type){
				switch (price_type) {
				case PRICE_CLOSE:
					return [&](int index)->double {
						return rates->close[index];
					};
				case PRICE_OPEN:
					return[&](int index)->double {
						return rates->open[index];
					};
				case PRICE_HIGH:
					return[&](int index)->double {
						return rates->open[index];
					};
				case PRICE_LOW:
					return[&](int index)->double {
						return rates->low[index];
					};
				case PRICE_MEDIAN:
					return[&](int index)->double {
						return (rates->high[index] + rates->low[index]) / 2;
					};
				case PRICE_TYPICAL:
					return[&](int index)->double {
						return (rates->high[index] + rates->low[index] + rates->close[index]) / 3;
					};
				case PRICE_WEIGHTED:
					return[&](int index)->double {
						return (rates->high[index] + rates->low[index] + rates->close[index] +rates->close[index]) / 4;
					};
				}
				return [&](int index)->double {
					return rates->close[index];
				};
			}

		protected:

			ChartData*		  rates; // бары в требуемом таймфрейме
			AbstractStrategy* manager;
			int				  outBufferLength; // длина буфера кэширования выходных результатов
			const int		  timeframe;
			std::vector<utils::CircularBuffer<double>*> buffers;

	};


}