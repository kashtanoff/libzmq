#pragma once
#if FALSE

#include <windows.h>

#include "../OrdersManager.cpp"


namespace fxc {

namespace indicator {

	class DefaultIndicator {

		public:

			DefaultIndicator(
				OrdersManager* const manager,
				const unsigned bufferLength,
				const double   delta,
				const double   deviation,
				const unsigned periodF1,
				const unsigned periodF2,
				const unsigned periodF3
			) :
				manager(manager),
				bufferLength(bufferLength),
				delta(delta),
				deviation(deviation),
				periodF1(periodF1),
				periodF2(periodF2),
				periodF3(periodF3)
			{
			}

			void compute() {
				auto data    = manager->getChartData( timeframe );
				auto dillers = manager->getDillers();

				double main_avr;
				double diffup, diffdn;
				double price = dillers->mpo - dillers->mpc;

				data.close[bufferLength] = dillers->mpc;
				data.high[bufferLength]  = max(dillers->mpc, data.high[bufferLength]);
				data.low[bufferLength]   = min(dillers->mpc, data.low[bufferLength]);

				int start = min(data->bars - counted, periodF2);
				if (start == periodF2) {
					prev_wd = prev_wu = delta * delta;
				}

				for (int i = start; i >= 0; i--) {
					main_avr = maw(&data, periodF1, i);
					diffup   = max(data.high[bufferLength - i] - main_avr, delta);
					diffdn   = max(main_avr - data.low[bufferLength - i], delta);
					diffup  *= diffup;
					diffdn  *= diffdn;

					wu = (prev_wu*(periodF2 - 1) + diffup) / periodF2;
					wd = (prev_wd*(periodF2 - 1) + diffdn) / periodF2;

					if (i > 0) {
						prev_wu = wu;
						prev_wd = wd;
					}
				}

				counted = data->bars;
				up_ind  = main_avr + deviation * sqrt(wu);
				dn_ind  = main_avr - deviation * sqrt(wd);

				if (periodF3) {
					for (int i = 1; i >= 0; i--) {
						maBuffer[i] = maw(&data, periodF3, i);
					}
				}
			}

			const std::vector< std::tuple<int, int> > getRequiredBuffers() {
				return buffers;
			}

			inline const double* const getUp() {
				return &up_ind;
			}

			inline const double* const getDown() {
				return &dn_ind;
			}

			inline const double* const getMaBuffer() {
				return maBuffer;
			}

		private:

			OrdersManager* const manager;

			const unsigned bufferLength;
			const double   delta;
			const double   deviation;
			const unsigned periodF1;
			const unsigned periodF2;
			const unsigned periodF3;

			std::vector< std::tuple<int, int> > buffers;

			double maBuffer[3];
			double up_ind, dn_ind;
			double wu, wd;
			double prev_wu, prev_wd;
			int    counted;

			inline double maw(ChartData* data, int period, int shift) {
				int j, k;
				double sum  = (period + 1) * data->close[bufferLength - shift];
				double sumw = period + 1;

				for (j = 1, k = period; j <= period; j++, k--) {
					sum  += k * data->close[bufferLength - (shift + j)];
					sumw += k;
				}

				return sum / sumw;
			}
			double ma(ChartData* data, int period, int shift) {
				int j, k;
				double sum  = (period + 1) * data->close[shift];
				double sumw = period + 1;

				for (j = 1, k = period; j <= period; j++, k--) {
					sum  += k * data->close[shift + j];
					sumw += k;
				}

				return sum / sumw;
			}

	};

}

}

#endif