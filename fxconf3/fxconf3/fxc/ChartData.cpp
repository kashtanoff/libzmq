#pragma once

namespace fxc {
	//Реализует кольцевой массив-тайм серию, в стиле mql4
	template <typename T>
	class CycloBuffer {
		public:
			CycloBuffer() {};
			alloc(unsigned size) {
				buffer = new T(buf_size = size);
				index = 0;
			}
			void add(T value) {
				index = index? index-1: buf_size-1;
				buffer[index] = value;
			}
			void update(T value) {
				buffer[index] = value;
			}
			T& operator[](unsigned i) {
				return(buffer[(index+i)%buf_size]);
			}
			~CycloBuffer() {
				delete []buffer;
			}
		private:
			T* buffer;
			int index;
			unsigned buf_size;
	};

	/*struct ChartData {
		double*	closes;
		double*	highs;
		double* lows;
		int		bars;
	};*/
	class ChartData {
	public:
		CycloBuffer<int> time;
		CycloBuffer<double> open;
		CycloBuffer<double> low;
		CycloBuffer<double> high;
		CycloBuffer<double> close;
		CycloBuffer<int> volume;
	
		ChartData()
		{};
		//Поздняя аллокация, поскольку длину буфера можно вычислить только после передачи параметров
		//Длина буфера - это количество последних баров, которые мы храним
		void initChartData(int buffSize) {
			time.alloc(buffSize);
			time[0] = 0;  //Для определения начальной инициализации
			open.alloc(buffSize);
			low.alloc(buffSize);
			high.alloc(buffSize);
			close.alloc(buffSize);
			volume.alloc(buffSize);
			counter = 0;
		};
		//Добавляет новый бар
		void addBar(int time_v, double open_v, double low_v, double high_v, double close_v, int volume_v) {
			time.add(time_v);
			open.add(open_v);
			low.add(low_v);
			high.add(high_v);
			close.add(close_v);
			volume.add(volume_v);
			counter++;
		};
		//Обновляет последний бар (бар к примеру, на таймфреме H1 собирается целый час, т.е. целый час он может менятся каждый тик, и нам нужны все эти изменения, а не только финальный образ)
		int updateBar(int time_v, double open_v, double low_v, double high_v, double close_v, int volume_v) {
			if (time[0] == time_v) {
				open.update(open_v);
				low.update(low_v);
				high.update(high_v);
				close.update(close_v);
				volume.update(volume_v);
			};
			return(time[0]);
		};
		//Возвращает время последнего бара
		int getLastTime() {
			return(time[0]);
		}
		int getCounter() {
			return counter;
		}
	private:
		int counter;
	};
}
/* На стороне mql внутри OnTick();
int t;
int price_buffer_length = PeriodF + PeriodF2 + 1;  //Формула может отличаться от индикатора к индикатору, поэтому можно ввести в абстрактный индикатор виртуальную функцию GetBufLen(); для последующего ее переопределения
if (bypass(ask, bid)) return;  //Тут выполняем препаратор и определяем (в случае оптимизации), нужно ли дальше работать в тике

int lasttime = c_refresh_chartdata(time[0], open[0], low[0], high[0], close[0], volume[0]);  //Обновление последнего бара
if (lasttime != time[0]) {  //Если это не обновление последнего бара
	int i = 0;
	if (!lasttime)  //Первичная инициализация
		i = price_buffer_length - 1;
	else
		while (i < price_buffer_length && lasttime != time[i])  //Поиск индекса последнего известного на стороне с++ бара
			i++;
	c_refresh_chartdata(time[i], open[i], low[i], high[i], close[i], volume[i]);  //Обновление предпоследнего бара, который в прошлый раз был последним
	i++;
	for (i; i>=0; i--)
		c_add_chartdata(time[i], open[i], low[i], high[i], close[i], volume[i]);
}
*/