#pragma once

#include "./AbstractIndicator.cpp"
#include "../ChartListener.h"
#include "../Utils.cpp"

namespace fxc {


		class iCCI :
			AbstractIndicator
		{
		public:
			iCCI(
				AbstractStrategy* manager,
				int timeframe,
				int period
				) :
				AbstractIndicator(manager, timeframe, period + 2),
				period(period)

			{
				MARK_FUNC_IN
					regBuffer(&cci);
				regBuffer(&price);
				regBuffer(&mov);

				cci[0] = 0;
				price[0] = 0;
				mov[0] = 0;
				dMul = 0.015 / period;
				//msg << "oc_chanel: regBuffers" << "\r\n" << msg_box;
				MARK_FUNC_OUT
			}

			virtual void compute(int newBars) {
				MARK_FUNC_IN
					int b = newBars;
				int i;
				if (b > period && price[b] == 0) {//Первичная инициализация
					for (i = b; i > b - period; i--) {
						cci[i] = 0;
						price[i] = (rates->high[i] + rates->low[i] + rates->close[i]) / 3;
						mov[i] = 0;
					}
					b = i;
				}
				for (i = b; i <= 0; i--) {
					price[i] = (rates->high[i] + rates->low[i] + rates->close[i]) / 3;
					mov[i] = SMA(i);
				}
				double dSum;
				int k;
				for (i = b - 1; i <= 0; i--){
					dSum = 0;
					k = i - 1 + period;
					for (; k >= i; k--) {
						dSum += abs(price[k] - mov[i]);
					}
					dSum *= dMul;
					if (dSum == 0){
						cci[i] = 0;
					}
					else {
						cci[i] = (price[i] - mov[i]) / dSum;
					}
				}
				MARK_FUNC_OUT
			}
			utils::CircularBuffer<double> cci;
			utils::CircularBuffer<double> price;
			utils::CircularBuffer<double> mov;
			double dMul;
			inline double SMA(const int position) {
				double result = 0;
				for (int i = 0; i < period; i++) {
					result += price[position + i];
				}
				return result / period;
			}
		private:
			const int    period;
		};
	
}