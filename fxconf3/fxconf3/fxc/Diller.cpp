#pragma once

#include <vector>
#include <functional>
#include <algorithm>

#include "debug/Debug.h"
#include "Order.cpp"

namespace fxc {

	class Diller {

		public:
			Diller*             opposite;
			int			        type;
			std::vector<Order*>	orders;
			Order*              ord_limit;
			Order*              ord_stop;
			double		        mpo;
			double		        mpc;
			int			        level;

			double		        step_peak  = 0;  //пик трейлинг шага
			double		        tp_peak    = 0;  //пик трейлинга тейкпрофита
			Order*              last       = nullptr;
			Order*              first      = nullptr;
			int			        prev_lvl   = 0;  //уровень предыдущей сетки
			double		        prev_lots  = 0;  //Размеры последних закрытых лотов
			int			        cur_av_lvl = 100;
			double		        _base_lot  = 0;

			Diller(int _type) {
				if (type = _type) {
					typeSign  = 1;
					bestPrice = [](double a, double b)             { return fmin(a, b); };
					comparer  = [](const Order* a, const Order* b) { return a->openprice < b->openprice; };
				}
				else {
					typeSign  = -1;
					bestPrice = [](double a, double b)             { return fmax(a, b); };
					comparer  = [](const Order* a, const Order* b) { return a->openprice > b->openprice; };
				}
			}

			void reset() {
				isSorted  = false;

				if (level) {
					prev_lvl  = level;
				}
				if (last) {
					prev_lots = last->lots;
				}

				orders.clear();
				ord_limit = nullptr;
				ord_stop  = nullptr;
				level     = 0;
			}

			inline void sortOrders() {
				 if (isSorted) {
					return;
				}

				if (level = orders.size()) {
					std::sort(orders.begin(), orders.end(), comparer);

					first = orders[0];
					last  = orders.back();
				}
				else {
					first = nullptr;
					last  = nullptr;
				}

				isSorted = true;
			}

			void addOrder(Order* order) {
				orders.push_back(order);
			}

			Order* getOrder(int index) {
				return orders[index];
			}

#pragma region Простые сервисные функции

			// Возвращает "лучшую цену" для типа
			std::function<double (double a, double b)> bestPrice;

			// Возвращает цену тейкпрофита
			double tp(double _open_price, double _tp) {
				return _open_price - _tp * typeSign;
			}

			// Возвращает цену стоплосса
			double sl(double _open_price, double _sl) {
				return _open_price + _sl * typeSign;
			}

			// Возвращает вес ордера
			double orderWeight(double open_price, double close_price, double _lots) {
				return (open_price - close_price) * _lots * typeSign;
			}

			// Возвращает дельту цены с учетом типа операции
			double delta(double low_price, double high_price) {
				return (low_price - high_price) * typeSign;
			}

			// Возвращает вес корзины
			double basketWeight(double _close_price, int _av_lvl = 100) {
				double weight = 0.0;
				for (int i = 0; i < level; i++) {
					weight += orderWeight(orders[i]->openprice, _close_price, orders[i]->lots);
					if (i + 1 >= _av_lvl) {
						return weight;
					}
				}
				return weight;
			}

			// Возвращает цену корзины на настоящий момент
			double basketCost() {
				MARK_FUNC_IN
				
				double cost = 0.0;
				for (int i = 0; i < level; i++) {
					cost += orders[i]->profit;
				}

				MARK_FUNC_OUT
				return cost;
			}

#pragma endregion


		private:

			bool isSorted = false;
			int  typeSign;
			std::function<bool(const Order* a, const Order* b)> comparer;

	};

}