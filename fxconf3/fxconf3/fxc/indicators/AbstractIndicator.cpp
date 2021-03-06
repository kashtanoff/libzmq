#pragma once

#include "../strat/AbstractStrategy.cpp" 

namespace fxc {
	
	class AbstractIndicator :
		ChartListener 
	{
		
		public:

			AbstractIndicator();
			AbstractIndicator(
				AbstractStrategy* manager,
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

			// ������������� ���������� �����, ���������� ������ ��� ����� �������� ���������
			virtual inline void listenChart() {
				MARK_FUNC_IN
				unsigned newBars = (rates->newBars > outBufferLength) ? outBufferLength : rates->newBars;
				//if (rates->newBars > outBufferLength) {
				//	msg << "listenChart to mach bars!!!\r\n" << msg_box;
				//}
				if (newBars) {
					for (auto& buffer : buffers) {
						buffer->skip(newBars);
					}
				}
				compute(newBars);
				MARK_FUNC_OUT
			}

			// ��������� ��� ����� ���������� ����������
			virtual void compute(int newBars) = 0;
		
			void regBuffer(utils::CircularBuffer<double>* buffer) {
				buffer->alloc(outBufferLength);
				buffers.push_back(buffer);
			}
			std::function<double(int)> getPriceMethod(int price_type){
				switch (price_type) {
				case PRICE_CLOSE:
					return [&](int index)->double {
						return rates->close[index];
					};
				case PRICE_OPEN:
					return[&](int index)->double {
						return rates->open[index];
					};
				case PRICE_HIGH:
					return[&](int index)->double {
						return rates->high[index];
					};
				case PRICE_LOW:
					return[&](int index)->double {
						return rates->low[index];
					};
				case PRICE_MEDIAN:
					return[&](int index)->double {
						return (rates->high[index] + rates->low[index]) / 2;
					};
				case PRICE_TYPICAL:
					return[&](int index)->double {
						return (rates->high[index] + rates->low[index] + rates->close[index]) / 3;
					};
				case PRICE_WEIGHTED:
					return[&](int index)->double {
						return (rates->high[index] + rates->low[index] + rates->close[index] +rates->close[index]) / 4;
					};
				}
				return [&](int index)->double {
					return rates->close[index];
				};
			}
			// MA on array
			double SimpleMA(const int position, const int period, utils::CircularBuffer<double> price)
			{
				double result = 0.0;
				double end = period + position;
				if (end > outBufferLength) throw std::out_of_range("SimpleMA wrong index");
				for (int i = position; i < end; i++) {
					result += price[i];
				}
				return result / period;
			}
			double Highest(const int position, const int period, utils::CircularBuffer<double> price)
			{
				double result = price[position];
				double end = period + position;
				if (end > outBufferLength) throw std::out_of_range("Highest wrong index");
				for (int i = position; i < end; i++) {
					result = (result < price[i])? price[i]: result;
				}
				return result;
			}
			double Lowest(const int position, const int period, utils::CircularBuffer<double> price)
			{
				double result = price[position];
				double end = period + position;
				if (end > outBufferLength) throw std::out_of_range("Lowest wrong index");
				for (int i = position; i < end; i++) {
					result = (result > price[i]) ? price[i] : result;
				}
				return result;
			}

		protected:

			ChartData*		  rates; // ���� � ��������� ����������
			AbstractStrategy* manager;
			int				  outBufferLength; // ����� ������ ����������� �������� �����������
			const int		  timeframe;
			std::vector<utils::CircularBuffer<double>*> buffers;

	};


}