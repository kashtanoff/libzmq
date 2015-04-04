#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace indicator {

	class AbstractIndicator :
		ChartListener 
	{
		
	public:
		AbstractIndicator();
		AbstractIndicator(
			OrdersManager* manager,
			int timeframe,
			int outBufferLength
		) :
			manager(manager),
			timeframe(timeframe),
			outBufferLength(outBufferLength)
		{
			manager->getTimeseries()->registerTimeframe(timeframe, outBufferLength, this);
			rates = manager->getChartData(timeframe);
		}
		//Автоматически вызываемый метод, вызывается каждый тик перед запуском стратегии
		inline void listenChart() {
			MARK_FUNC_IN
			if (rates->newBars)
				for (auto& buffer : buffers)
					buffer->skip(rates->newBars);
			compute(rates->newBars);
			MARK_FUNC_OUT
		}
		//Заготовка под метод вычисления индикатора
		virtual void compute(int newBars) = 0;
		void regBuffer(utils::CircularBuffer<double>* buffer) {
			buffer->alloc(outBufferLength);
			buffers.push_back(buffer);
		}
	protected:
		ChartData*		rates; // бары в требуемом таймфрейме
		OrdersManager*	manager;
		int				outBufferLength; // длина буфера кэширования выходных результатов
		const int		timeframe;
		std::vector<utils::CircularBuffer<double>*> buffers;
	};

}

}