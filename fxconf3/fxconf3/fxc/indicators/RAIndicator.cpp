#pragma once

#include "./AbstractIndicator.cpp"
#include "../ChartListener.h"
#include "../Utils.cpp"

namespace fxc {

namespace ra_indicator {

	class RAIndicator :  
		AbstractIndicator
	{
		public:
			RAIndicator(
				AbstractStrategy* manager,
				int timeframe,
				int period1,
				int period2,
				double deviation,
				double mindev
			) :
				AbstractIndicator(manager, timeframe, period1 + period2 + 2),
				period1(period1),
				period2(period2),
				deviation(deviation),
				mindev(mindev)
				
			{
				MARK_FUNC_IN
				regBuffer(&up);
				regBuffer(&ups);
				regBuffer(&down);
				regBuffer(&downs);
				regBuffer(&middle);

				ups[0]   = 0;
				ups[1]   = 0;
				downs[0] = 0;
				downs[1] = 0;
				//msg << "oc_chanel: regBuffers" << "\r\n" << msg_box;
				MARK_FUNC_OUT
			}

			virtual void compute(int newBars) {
				MARK_FUNC_IN
					try {
				int b = min(newBars, outBufferLength - period1 - 1);
				if (b == 0 && manager->bid < rates->high[0] && manager->bid > rates->low[0]) {
					// ���� �� ���������� ���� �� ��������� ����������, ��� ������ �������������
					return;
				}
				for (int i = b; i >= 0; i--) {
					double sumw = period1 + 1;
					double sum  = sumw * rates->close[i];
					for (int j = 1, k = period1; j <= period1; j++, k--) {
						sum  += k * rates->close[i + j];
						sumw += k;
					}
					middle[i] = sum / sumw;

					double upDiff = rates->high[i] - middle[i];
					double dnDiff = middle[i] - rates->low[i];
					upDiff   = max(upDiff, mindev);
					dnDiff   = max(dnDiff, mindev);
					upDiff  *= upDiff;
					dnDiff  *= dnDiff;
					ups[i]   = (ups[i + 1]   * (period2 - 1) + upDiff) / period2;
					downs[i] = (downs[i + 1] * (period2 - 1) + dnDiff) / period2;
					up[i]    = middle[i] + sqrt(ups[i])   * deviation;
					down[i]  = middle[i] - sqrt(downs[i]) * deviation;
				}
				}
				catch (const std::exception& ex) {
					fxc::msg << "!> ERROR @ RAIndicator compute(): " << ex.what() << "\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}
				catch (const std::string& ex) {
					fxc::msg << "!> ERROR @ RAIndicator compute(): " << ex << "\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}
				catch (...) {
					fxc::msg << "!> ERROR @ RAIndicator compute(): [undefined type]\r\n" << fxc::msg_box;
					fxc::msg << STACK_TRACE << fxc::msg_box;
				}
				MARK_FUNC_OUT
			}
			utils::CircularBuffer<double> up;
			utils::CircularBuffer<double> ups;
			utils::CircularBuffer<double> down;
			utils::CircularBuffer<double> downs;
			utils::CircularBuffer<double> middle;
		private:
			const int    period1;
			const int    period2;
			const double deviation;
			const double mindev;
	};
}
}