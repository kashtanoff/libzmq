#pragma once

#include "./AbstractIndicator.cpp"
#include "../ChartListener.h"
#include "../Utils.cpp"

namespace fxc {

namespace indicator {

	class RAIndicator : 
		public AbstractIndicator,
		public ChartListener
	{

		public:

			RAIndicator(
				OrdersManager* manager,
				int timeframe,
				int period1,
				int period2,
				double deviation,
				double mindev
			) :
				timeframe(timeframe),
				period1(period1),
				period2(period2),
				deviation(deviation),
				mindev(mindev),
				outBufferLength(period1 + period2 + 2)
			{
				MARK_FUNC_IN
				manager->getTimeseries()->registerTimeframe(timeframe, outBufferLength, this);
				rates = manager->getChartData(timeframe);

				MARK_FUNC_IN
				up.alloc(outBufferLength);
				ups.alloc(outBufferLength);
				down.alloc(outBufferLength);
				downs.alloc(outBufferLength);
				middle.alloc(outBufferLength);

				ups[0]   = 0;
				ups[1]   = 0;
				downs[0] = 0;
				downs[1] = 0;
				MARK_FUNC_OUT
				MARK_FUNC_OUT
			}

			virtual inline void listenChart() {
				MARK_FUNC_IN
				compute();
				MARK_FUNC_OUT
			}

			virtual void compute() {
				MARK_FUNC_IN
				int b = min(rates->newBars, period2);
				if (b) {
					up.skip(b);
					ups.skip(b);
					down.skip(b);
					downs.skip(b);
					middle.skip(b);
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
					upDiff = max(upDiff, mindev);
					dnDiff = max(dnDiff, mindev);

					upDiff  *= upDiff;
					dnDiff  *= dnDiff;
					ups[i]   = (ups[i + 1]   * (period2 - 1) + upDiff) / period2;
					downs[i] = (downs[i + 1] * (period2 - 1) + dnDiff) / period2;
					up[i]    = middle[i] + sqrt(ups[i])   * deviation;
					down[i]  = middle[i] - sqrt(downs[i]) * deviation;

				}
				MARK_FUNC_OUT
			}
		
			utils::CircularBuffer<double> up;
			utils::CircularBuffer<double> ups;
			utils::CircularBuffer<double> down;
			utils::CircularBuffer<double> downs;
			utils::CircularBuffer<double> middle;

		private:

			ChartData* rates; // ���� � ��������� ����������

			const int    outBufferLength; // ����� ������ ����������� �������� �����������
			const int    timeframe;
			const int    period1;
			const int    period2;
			const double deviation;
			const double mindev;

	};

}

}