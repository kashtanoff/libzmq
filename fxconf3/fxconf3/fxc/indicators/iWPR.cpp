#pragma once

#include "./AbstractIndicator.cpp"
#include "../ChartListener.h"
#include "../Utils.cpp"

namespace fxc {


	class iWPR :
		AbstractIndicator
	{
	public:
		utils::CircularBuffer<double> wpr;

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
			DEBUG_TRY
				MARK_FUNC_IN
				int b = min(newBars, outBufferLength-period -1);
				double dMaxHigh;
				double dMinLow;
				double d;
				for (int i = b; i >= 0; i--) {
					dMaxHigh = Highest(i, period, rates->high);
					dMinLow = Lowest(i, period, rates->low);
					if (dMaxHigh - dMinLow <= 0) {
						throw std::logic_error("WPR delta wrong");
					}
					wpr[i] = -100 * (dMaxHigh - rates->close[i]) / (dMaxHigh - dMinLow);
				}
				MARK_FUNC_OUT
			DEBUG_CATCH("iWPR compute")
		}
		

	private:
		const int    period;
	};

}