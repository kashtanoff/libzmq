#pragma once

#include <windows.h>

#include "../Parameters.cpp"
#include "../OrdersManager.cpp"

namespace fxc {

namespace indicator {

	class DefaultIndicator {

		public:

			DefaultIndicator(const Parameters* const params, OrdersManager* const manager) : 
				params(params),
				manager(manager) 
			{
			}

			void compute() {
				auto data    = manager->getChartData();
				auto dillers = manager->getDillers();

				double main_avr;
				double diffup, diffdn;
				double price = dillers->mpo - dillers->mpc;

				data->closes[params->input_buf_len] = dillers->mpc;
				data->highs[params->input_buf_len]  = max(dillers->mpc, data->highs[params->input_buf_len]);
				data->lows[params->input_buf_len]   = min(dillers->mpc, data->lows[params->input_buf_len]);

				int start = min(data->bars - counted, params->input_periodf2);
				if (start == params->input_periodf2) {
					prev_wd = prev_wu = params->input_delta * params->input_delta;
				}

				for (int i = start; i >= 0; i--) {
					main_avr = maw(data, params->input_period, i);
					diffup   = max(data->highs[params->input_buf_len - i] - main_avr, params->input_delta);
					diffdn   = max(main_avr - data->lows[params->input_buf_len - i], params->input_delta);
					diffup  *= diffup;
					diffdn  *= diffdn;

					wu = (prev_wu*(params->input_periodf2 - 1) + diffup) / params->input_periodf2;
					wd = (prev_wd*(params->input_periodf2 - 1) + diffdn) / params->input_periodf2;

					if (i > 0) {
						prev_wu = wu;
						prev_wd = wd;
					}
				}

				counted = data->bars;
				up_ind  = main_avr + params->input_deviation * sqrt(wu);
				dn_ind  = main_avr - params->input_deviation * sqrt(wd);

				if (params->input_periodf3) {
					for (int i = 1; i >= 0; i--) {
						maBuffer[i] = maw(data, params->input_periodf3, i);
					}
				}
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

			const Parameters* const params;
			OrdersManager* const manager;

			double maBuffer[3];
			double up_ind, dn_ind;
			double wu, wd;
			double prev_wu, prev_wd;
			int    counted;

			inline double maw(ChartData* data, int period, int shift) {
				int j, k;
				double sum  = (period + 1) * data->closes[params->input_buf_len - shift];
				double sumw = period + 1;

				for (j = 1, k = period; j <= period; j++, k--) {
					sum  += k * data->closes[params->input_buf_len - (shift + j)];
					sumw += k;
				}

				return sum / sumw;
			}

	};

}

}