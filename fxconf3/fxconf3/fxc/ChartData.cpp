#pragma once

#include <unordered_map>
#include <algorithm>
#include "./fxc.h"
#include "./ChartListener.h"
#include "./Utils.cpp"
#include "../MqlUtils.cpp"

namespace fxc {

	class ChartData {

		public:

			unsigned newBars = 0;

			utils::CircularBuffer<__time64_t> time;
			utils::CircularBuffer<double>     open;
			utils::CircularBuffer<double>     low;
			utils::CircularBuffer<double>     high;
			utils::CircularBuffer<double>     close;
			utils::CircularBuffer<__int64>    volume;

			ChartData() {
			}

			// ������� ���������, ��������� ����� ������ ����� ��������� ������ ����� �������� ����������
			// ����� ������ - ��� ���������� ��������� �����, ������� �� ������
			void resize(int buffSize) {
				if (getSize()) {
					time.alloc(buffSize);
				}
				else {
					time.alloc(buffSize);
					time.update(0);
				}
				low.alloc(buffSize);
				high.alloc(buffSize);
				open.alloc(buffSize);
				close.alloc(buffSize);
				volume.alloc(buffSize);
			}

			void update(MqlRates* pointer, int length) {
				MARK_FUNC_IN
				newBars = 0;
				
				if ((pointer + length - 1)->time == time[0]) {
					MARK_FUNC_OUT
					return;
				}
				MARK_FUNC_OUT
				
				MARK_FUNC_IN
				int i;
				MqlRates* data;

				for (i = length - 2; i >= 0; i--) {
					data = pointer + i;
					if (data->time == time[0]) {
						time.update(data->time);
						low.update(data->low);
						high.update(data->high);
						open.update(data->open);
						close.update(data->close);
						volume.update(data->tick_volume);
						break;
					}
				}
				MARK_FUNC_OUT

				MARK_FUNC_IN
				i++;
				newBars = length - i;
				for (; i < length; ++i) {
					data = pointer + i;
					time.add(data->time);
					low.add(data->low);
					high.add(data->high);
					open.add(data->open);
					close.add(data->close);
					volume.add(data->tick_volume);
				}
				MARK_FUNC_OUT
			}

			inline const int getSize() {
				return time.getSize();
			}

	};

	class TimeSeries { 

		public:

			inline void timeSeriesReset() {
				for (auto& pair : _chartData) {
					pair.second->newBars = 0;
				}
			}
			~TimeSeries() {
				for (auto& pair : _chartData) {
					delete pair.second;
				}
			}
			void updateFirst(const double ask, const double bid) {
				MARK_FUNC_IN
				for (auto& pair : _chartData) {
					pair.second->high[0]  = max(pair.second->high[0], bid);
					pair.second->low[0]   = min(pair.second->low[0], bid);
					pair.second->close[0] = bid;
				}
				MARK_FUNC_OUT
			}

			void registerTimeframe(const int timeframe, const int length, fxc::ChartListener* const listener) {
				MARK_FUNC_IN
				registerTimeframe(timeframe, length);
				registerListener(timeframe, listener);
				MARK_FUNC_OUT
			}

			void registerTimeframe(const int timeframe, const int length) {
				MARK_FUNC_IN
				if (!_chartData.count(timeframe)) {
					_chartData[timeframe] = new ChartData();
					_timeframes.push_back(timeframe);
					std::sort(_timeframes.begin(), _timeframes.end());
				}
				if (_chartData[timeframe]->getSize() < length) {
					_chartData[timeframe]->resize(length);
				}
				MARK_FUNC_OUT
			}

			inline void registerListener(const int timeframe, fxc::ChartListener* const listener) {
				MARK_FUNC_IN
				_listeners[timeframe].push_back(listener);
				MARK_FUNC_OUT
			}

			inline void invokeListeners() {
				MARK_FUNC_IN
				for (auto& pair : _listeners) {
					for (auto& listener : pair.second) {
						listener->listenChart();
					}
				}
				MARK_FUNC_OUT
			}

			inline const std::vector<int> getTimeframes() {
				return _timeframes;
			}

			inline fxc::ChartData* getChartData(const int timeframe) {
				return _chartData[timeframe];
			}

		private:

			std::vector<int>                                         _timeframes;
			std::unordered_map<int, ChartData*>                       _chartData;
			std::unordered_map<int, std::vector<fxc::ChartListener*>> _listeners;

	};

}