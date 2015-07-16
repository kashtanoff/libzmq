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
				AbstractIndicator(manager, timeframe, period*2 + 2),
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
					try {
					int b = newBars;
					//msg << "iCCI newBars: " << newBars << "\r\n" << msg_box;

					int i;
					if (b == outBufferLength) {//Первичная инициализация
						//msg << "iCCI first init\r\n" << msg_box;
						b--;
						//msg << "iCCI b1: " << b << "\r\n" << msg_box;
						for (i = b; i > b - period; i--) {
							cci[i] = 0;
							price[i] = (rates->high[i] + rates->low[i] + rates->close[i]) / 3;
							mov[i] = 0;
						}
						b = i;
					}
					//msg << "iCCI b2: " << b << "\r\n" << msg_box;
					for (i = b; i >= 0; i--) {
						price[i] = (rates->high[i] + rates->low[i] + rates->close[i]) / 3;
						mov[i] = SimpleMA(i, period, price);
					}
					double dSum;
					int k;
					b = min(b, outBufferLength - period - period - 1);
					msg << "iCCI b3: " << b << "\r\n" << msg_box;
					for (i = b; i >= 0; i--){
						dSum = 0;
						//msg << "iCCI b4: " << i - 1 + period << "\r\n" << msg_box;
						for (k = i - 1 + period; k >= i; k--) {
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
					//msg << "iCCI end compute\r\n" << msg_box;
				}
				catch (const std::exception& ex) {
					fxc::msg << "!> ERROR @ cci compute(): " << ex.what() << "\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}
				catch (const std::string& ex) {
					fxc::msg << "!> ERROR @ cci compute(): " << ex << "\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}
				catch (...) {
					fxc::msg << "!> ERROR @ cci compute(): [undefined type]\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}				MARK_FUNC_OUT
			}
			utils::CircularBuffer<double> cci;
			utils::CircularBuffer<double> price;
			utils::CircularBuffer<double> mov;
			double dMul;

		private:
			const int    period;
		};
	
}