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

			// Поздняя аллокация, поскольку длину буфера можно вычислить только после передачи параметров
			// Длина буфера - это количество последних баров, которые мы храним
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

			void update(MqlRates* pointer, unsigned length) {
				MARK_FUNC_IN
				newBars = 0;

				if ((pointer + length - 1)->time == time[0]) {
					return;
				}

				int i;
				MqlRates* data;

				MARK_FUNC_IN
				for (i = length - 2; i >= 0; --i) {
					data = pointer + i;
					if (data->time == time[0]) {
						MARK_FUNC_IN
						time.update(data->time);
						low.update(data->low);
						high.update(data->high);
						open.update(data->open);
						close.update(data->close);
						volume.update(data->tick_volume);

						newBars = length - ++i;

						MARK_FUNC_IN
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
						MARK_FUNC_OUT
						MARK_FUNC_OUT
						return;
					}
				}
				MARK_FUNC_OUT

				MARK_FUNC_IN
				newBars = length;

				for (i = 0; i < length; ++i) {
					data = pointer + i;
					
					time.add(data->time);
					low.add(data->low);
					high.add(data->high);
					open.add(data->open);
					close.add(data->close);
					volume.add(data->tick_volume);
				}
				MARK_FUNC_OUT
				MARK_FUNC_OUT
			}
			
			inline const unsigned getSize() {
				return time.getSize();
			}

	};

	class TimeSeries {

		public:

			TimeSeries() {
			}

			void reset() {
				for (auto& pair : _chartData) {
					pair.second->newBars = 0;
				}
			}

			void updateFirst(const double ask, const double bid) {
				for (auto& pair : _chartData) {
					pair.second->high[0]  = max(pair.second->high[0], bid);
					pair.second->low[0]   = min(pair.second->low[0], bid);
					pair.second->close[0] = bid;
				}
			}

			void registerTimeframe(const int timeframe, const unsigned length, fxc::ChartListener* const listener) {
				MARK_FUNC_IN
				registerTimeframe(timeframe, length);
				registerListener(timeframe, listener);
				MARK_FUNC_OUT
			}

			void registerTimeframe(const int timeframe, const unsigned length) {
				MARK_FUNC_IN
				MARK_FUNC_IN
				if (!_chartData.count(timeframe)) {
					_chartData[timeframe] = new ChartData();
					_timeframes.push_back(timeframe);
					std::sort(_timeframes.begin(), _timeframes.end());
				}
				MARK_FUNC_OUT
				MARK_FUNC_IN
				if (_chartData[timeframe]->getSize() < length) {
					_chartData[timeframe]->resize(length);
				}
				MARK_FUNC_OUT
				MARK_FUNC_OUT
			}

			void registerListener(const int timeframe, fxc::ChartListener* const listener) {
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