#pragma once

#include "../strat/AbstractStrategy.cpp"

namespace fxc {

namespace indicator {

	class AbstractIndicator :
		ChartListener 
	{
		
	public:
		AbstractIndicator();
		AbstractIndicator(
			TimeSeries* manager,
			int timeframe,
			int outBufferLength
		) :
			manager(manager),
			timeframe(timeframe),
			outBufferLength(outBufferLength)
		{
			manager->registerTimeframe(timeframe, outBufferLength, this);
			rates = manager->getChartData(timeframe);
		}
		//������������� ���������� �����, ���������� ������ ��� ����� �������� ���������
		inline void listenChart() {
			MARK_FUNC_IN
			if (rates->newBars)
				for (auto& buffer : buffers)
					buffer->skip(rates->newBars);
			compute(rates->newBars);
			MARK_FUNC_OUT
		}
		//��������� ��� ����� ���������� ����������
		virtual void compute(int newBars) = 0;
		void regBuffer(utils::CircularBuffer<double>* buffer) {
			buffer->alloc(outBufferLength);
			buffers.push_back(buffer);
		}
	protected:
		ChartData*		rates; // ���� � ��������� ����������
		TimeSeries*		manager;
		int				outBufferLength; // ����� ������ ����������� �������� �����������
		const int		timeframe;
		std::vector<utils::CircularBuffer<double>*> buffers;
	};

}

}