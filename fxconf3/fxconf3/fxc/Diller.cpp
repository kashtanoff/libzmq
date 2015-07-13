#pragma once

#include <vector>
#include <functional>
#include <algorithm>

#include "debug/Debug.h"
#include "Order.cpp"

namespace fxc {
	class CascadRule{
	public:
		bool rule_done;
		//int type;
		CascadRule* next;
		std::function<bool()> rule;


		CascadRule(std::function<bool()> rule, CascadRule* next = nullptr) :
			//type(type),
			rule(rule),
			next(next)
		{
			rule_done = false;
		}
		~CascadRule() {
			if (next != nullptr)
				delete next;
		}
		//virtual bool rule() { return false; }
		bool check() {
			if (rule_done) {
				if (next->check()) {
					rule_done = false;
					return true;
				}
			}
			else {
				if (rule()) {
					if (next != nullptr) {
						if (next->check()) {
							return true;
						}
						rule_done = true;
					}
					else {
						return true;
					}
				}
			}
			return false;
		}
	};

	class Diller {

		public:
			Diller*             opposite;				//указатель на противоположный диллер
			int			        type;					//тип диллера: 0-BUY, 1-SELL
			std::vector<Order*>	orders;					//рыночные ордера диллера
			Order*              ord_limit;				//последний лимитный ордер диллера
			Order*              ord_stop;				//последний стоп оредер диллера
			double		        mpo;					//цена открытия для операций диллера
			double		        mpc;					//цена закрытия для операций диллера
			int			        level;					//количество ордеров диллера в рынке 
			double				total_lots;				//общая лотность диллера в рынке
			double				open_dd;				//открытая просадка по диллеру

			int					trail_in_mode;
			CascadRule*			c_rule;
			double		        trail_in_peak	= 0;	//пик трейлинг входа
			double				trail_in_delta	= 0;
			std::function<bool()>	trail_in_rule;
			std::function<void()>	trail_in_callback;
			double		        trail_out_peak	= 0;	//пик трейлинга выхода
			double				trail_out_delta = 0;
			std::function<void()>	trail_out_callback;
			__time64_t	ban_bar;

			Order*              last       = nullptr;	//последний ордер диллера в рынке
			Order*              first      = nullptr;	//первый ордер диллера в рынке
			int			        prev_lvl   = 0;			//уровень предыдущей сетки
			double		        prev_lots  = 0;			//Размеры последних закрытых лотов

			int			        cur_av_lvl = 100;
			double		        base_lot  = 0;

			std::string			open_reason;			//для отладки, трейс причины открытия

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
				open_reason = "";
				trail_in_mode = 0;
			}
			~Diller() {
				if (c_rule != nullptr) {
					delete c_rule;
				}
			}
			//Сброс диллеров в начале тика
			void reset() {
				if (level) {
					prev_lvl  = level;
				}
				if (last) { 
					prev_lots = last->lots;
				}
				orders.clear();
				ord_limit	= nullptr;
				ord_stop	= nullptr;
				level		= 0;
				total_lots	= 0;
				open_dd		= 0;
			}

			inline void sortOrders() {
				if (level = orders.size()) {
					std::sort(orders.begin(), orders.end(), comparer);

					first = orders[0];
					last  = orders.back();
				}
				else {
					first = nullptr;
					last  = nullptr;
				}
			}

			inline void addOrder(Order* order) {
				orders.push_back(order);
				open_dd += order->profit;
				total_lots += order->lots;
			}

			inline Order* getOrder(int index) {
				return orders[index];
			}
			inline void trail_in_init(double delta) {
				trail_in_delta = delta;
				trail_in_peak = 0;
			}
			inline void trail_in_reset(){
				trail_in_peak = 0;
			}
			inline void trail_in_start() {
				trail_in_peak = mpo;
			}
			inline bool trail_in_stop() {
				if (trail_in_peak == 0)
					return false;
				if (type) { //Продажи
					if (trail_in_peak - mpo >= trail_in_delta) {
						trail_in_peak = 0;
						return true;
					}
					else {
						trail_in_peak = fmax(trail_in_peak, mpo);
						return false;
					}
				}
				else {
					if (mpo - trail_in_peak >= trail_in_delta) {
						trail_in_peak = 0;
						return true;
					}
					else {
						trail_in_peak = fmin(trail_in_peak, mpo);
						return false;
					}
				}
				return false;
			}
			inline double check_peak(double rollback) {
				if (delta(trail_in_peak, mpo) >= rollback) {
					trail_in_mode = 0;
					return true;
				}
				else {
					if (type) {
						trail_in_peak = fmax(trail_in_peak, mpo);
					}
					else {
						trail_in_peak = fmin(trail_in_peak, mpo);
					}
				}
				return false;
			}

#pragma region Простые сервисные функции

			// Возвращает "лучшую цену" для типа
			std::function<double (double a, double b)> bestPrice;

			// Возвращает цену тейкпрофита
			inline double tp(double _open_price, double _tp) {
				return _open_price - _tp * typeSign;
			}

			// Возвращает цену стоплосса
			inline double sl(double _open_price, double _sl) {
				return _open_price + _sl * typeSign;
			}

			// Возвращает вес ордера
			inline double orderWeight(double open_price, double close_price, double _lots) {
				return (open_price - close_price) * _lots * typeSign;
			}
			inline double orderWeight(Order* order, double close_price) {
				return (order->openprice - close_price) * order->lots * typeSign;
			}

			// Возвращает дельту цены с учетом типа операции buy(high-low)
			inline double delta(double low_price, double high_price) {
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
			int  typeSign;
			std::function<bool(const Order* a, const Order* b)> comparer;

	};

}