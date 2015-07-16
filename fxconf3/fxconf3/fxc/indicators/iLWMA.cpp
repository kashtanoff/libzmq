#pragma once

#include "./AbstractIndicator.cpp"
#include "../ChartListener.h"
#include "../Utils.cpp"

namespace fxc {

	class iLWMA :  
		AbstractIndicator
	{
		public:
			iLWMA(
				AbstractStrategy* manager,
				int timeframe,
				int period,
				int price_type
			) :
				AbstractIndicator(manager, timeframe, period + 3),
				period(period),
				price_type(price_type)
			{
				MARK_FUNC_IN
				regBuffer(&ma);
				price = getPriceMethod(price_type);
				MARK_FUNC_OUT
			}

			virtual void compute(int newBars) {
				MARK_FUNC_IN
					try{
				int b = min(newBars, outBufferLength - period - 1);
				for (int i = b; i >= 0; i--) {
					double sumw = period + 1;
					double sum  = sumw * price(i);
					for (int j = 1, k = period; j <= period; j++, k--) {
						sum  += k * price(i + j);
						sumw += k;
					}
					ma[i] = sum / sumw;
				}
				}
				catch (const std::exception& ex) {
					fxc::msg << "!> ERROR @ iLWMA compute(): " << ex.what() << "\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}
				catch (const std::string& ex) {
					fxc::msg << "!> ERROR @ iLWMA compute(): " << ex << "\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}
				catch (...) {
					fxc::msg << "!> ERROR @ iLWMA compute(): [undefined type]\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}

				MARK_FUNC_OUT
			}
			
			utils::CircularBuffer<double> ma;
		private:
			const int    period;
			const int	 price_type;
			std::function<double(int)> price;
	};
}