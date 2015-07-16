#pragma once

#include "./AbstractIndicator.cpp"
#include "../ChartListener.h"
#include "../Utils.cpp"

namespace fxc {


	class iWPR :
		AbstractIndicator
	{
	public:
		iWPR(
			AbstractStrategy* manager,
			int timeframe,
			int period
			) :
			AbstractIndicator(manager, timeframe, period + 2),
			period(period)

		{
			MARK_FUNC_IN
				regBuffer(&wpr);

			wpr[0] = 0;
			MARK_FUNC_OUT
		}

		virtual void compute(int newBars) {
			MARK_FUNC_IN
			try {
				int b = min(newBars, outBufferLength-period -1);
				double dMaxHigh;
				double dMinLow;
				double d;
				//msg << "b=" << b << "\r\n" << msg_box;
				for (int i = b; i >= 0; i--) {
					dMaxHigh = Highest(i, period, rates->high);
					dMinLow = Lowest(i, period, rates->low);
					if (dMaxHigh - dMinLow <= 0) {
						msg << "delta wrong: " << dMaxHigh - dMinLow << "\r\n" << msg_box;
						throw "WPR delta wrong";
					}
					wpr[i] = -100 * (dMaxHigh - rates->close[i]) / (dMaxHigh - dMinLow);
				}
			}
			catch (const std::exception& ex) {
				fxc::msg << "!> ERROR @ wpr compute(): " << ex.what() << "\r\n" << fxc::msg_box;
				fxc::msg << STACK_TRACE << fxc::msg_box;
			}
			catch (const std::string& ex) {
				fxc::msg << "!> ERROR @ wpr compute(): " << ex << "\r\n" << fxc::msg_box;
				fxc::msg << STACK_TRACE << fxc::msg_box;
			}
			catch (...) {
				fxc::msg << "!> ERROR @ wpr compute(): [undefined type]\r\n" << fxc::msg_box;
				fxc::msg << STACK_TRACE << fxc::msg_box;
			}				
			MARK_FUNC_OUT
		}
		utils::CircularBuffer<double> wpr;

	private:
		const int    period;
	};

}