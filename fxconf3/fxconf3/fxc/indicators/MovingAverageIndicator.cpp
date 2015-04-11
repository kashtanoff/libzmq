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
			// ��� timeframe ����� ������ ������������ ����������� ��� �������� �������������
			// � �������� ����������� ���������� �������� ����� RegIndicator(int timeframe, int period, delegate compute=null);
			// ����� ������ ��� � ������ ���� ���������� ��������������� �������� ��� ������������������ ������ compute, ��� ���������� �����������
			// ��� ��������� ����������� ����������, ��������� ������ � ������ ������������� ��������
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
				// ��� ���� ����������� ����������� ����������� � �� �������� � ���������� ������ �����������
				// ���� �� �� ����� ���������� ����������, �� �� ������ compute

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
				// ����������� ����������, ��� ��� �������, ����� � ������ ���� �� ����� ������������ ���������� ��������� ���
				for (int i = 0; i < outBufferLength; i++) {
					outbuffer[i] = value(i);
				}
				MARK_FUNC_OUT
			}

			// ��� ��� �������, ����� �� �� ����� ��������� �������� compute
			// �� ���������� �������� �������
			const double value(const unsigned shift = 0) {
				double sum  = (period + 1) * rates->close[shift];
				double sumw = period + 1;
				for (int j = 1, k = period; j <= period; j++, k--) {
					sum  += k * rates->close[shift + j];
					sumw += k;
				}
				return sum / sumw;
			}

			// ��������������, ����� ����� ���� ������������ ����������� � ��������� ��� ma[1], ��� ma = new MovingAverage(manager, 3, Parameters.PeriodF3, 14);
			inline double& operator[](const unsigned i) {
				return outbuffer[i];
			}

		private:

			ChartData* rates;                             // ���� � ��������� ����������
			fxc::utils::CircularBuffer<double> outbuffer; // ����� ��� ����������� �������� �����������

			const int outBufferLength; // ����� ������ ����������� �������� �����������
			const int timeframe;
			const int period;

	};

}

}