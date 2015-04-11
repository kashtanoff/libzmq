#pragma once

#include <windows.h>

#include "./AbstractIndicator.cpp"
#include "../TradeManager.cpp"
#include "../OrdersManager.cpp"
#include "../ChartListener.h"
#include "../Utils.cpp"

namespace fxc {

namespace indicator {

	class MovingAverageIndicator : 
		public AbstractIndicator
	{

		public:

			// what to do:
			// для timeframe можно ввести перечисление таймфреймов для удобства использования
			// в менеджер индикаторов необходимо добавить метод RegIndicator(int timeframe, int period, delegate compute=null);
			// также каждый раз в начале тика необходимо последовательно вызывать все зарегистрированные методы compute, для обновления индикаторов
			// при повторной регистрации таймфрейма, обновляем период в пользу максимального значения
			MovingAverageIndicator(
				TimeSeries* manager,
				const int outBufferLength,
				const int timeframe, 
				const int period
			) :
				outBufferLength(outBufferLength),
				timeframe(timeframe),
				period(period)
			{
				// так идет регистрация необходимых таймфреймов и их периодов и сохранение метода вычислителя
				// если мы не хотим постоянных вычислений, то не ставим compute

				if (outBufferLength) {
					outbuffer.alloc(outBufferLength);
				}
			}

			const std::vector< std::tuple<int, int> > getRequiredBuffers() {
				std::vector< std::tuple<int, int> > v;
				v.push_back( std::make_tuple(timeframe, period + max(1, outBufferLength)) );
				return v;
			}

			virtual inline void listenChart() {
				MARK_FUNC_IN
				compute();
				MARK_FUNC_OUT
			}

			virtual void compute() {
				MARK_FUNC_IN
				// кеширование вычислений, для тех случаев, когда в рамках тика мы хотим использовать результаты несколько раз
				for (int i = 0; i < outBufferLength; i++) {
					outbuffer[i] = value(i);
				}
				MARK_FUNC_OUT
			}

			// для тех случаев, когда мы не хотим регулярно вызывать compute
			// не использует выходные буфферы
			const double value(const unsigned shift = 0) {
				double sum  = (period + 1) * rates->close[shift];
				double sumw = period + 1;
				for (int j = 1, k = period; j <= period; j++, k--) {
					sum  += k * rates->close[shift + j];
					sumw += k;
				}
				return sum / sumw;
			}

			// украшательство, чтобы можно было использовать конструкцию в стратегии как ma[1], где ma = new MovingAverage(manager, 3, Parameters.PeriodF3, 14);
			inline double& operator[](const unsigned i) {
				return outbuffer[i];
			}

		private:

			ChartData* rates;                             // бары в требуемом таймфрейме
			fxc::utils::CircularBuffer<double> outbuffer; // буфер для кэширования выходных результатов

			const int outBufferLength; // длина буфера кэширования выходных результатов
			const int timeframe;
			const int period;

	};

}

}