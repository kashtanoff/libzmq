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
			Diller*             opposite;				//��������� �� ��������������� ������
			int			        type;					//��� �������: 0-BUY, 1-SELL
			std::vector<Order*>	orders;					//�������� ������ �������
			Order*              ord_limit;				//��������� �������� ����� �������
			Order*              ord_stop;				//��������� ���� ������ �������
			double		        mpo;					//���� �������� ��� �������� �������
			double		        mpc;					//���� �������� ��� �������� �������
			int			        level;					//���������� ������� ������� � ����� 
			double				total_lots;				//����� �������� ������� � �����
			double				open_dd;				//�������� �������� �� �������

			int					trail_in_mode;
			CascadRule*			c_rule;
			double		        trail_in_peak	= 0;	//��� �������� �����
			double				trail_in_delta	= 0;
			std::function<bool()>	trail_in_rule;
			std::function<void()>	trail_in_callback;
			double		        trail_out_peak	= 0;	//��� ��������� ������
			double				trail_out_delta = 0;
			std::function<void()>	trail_out_callback;
			__time64_t	ban_bar;

			Order*              last       = nullptr;	//��������� ����� ������� � �����
			Order*              first      = nullptr;	//������ ����� ������� � �����
			int			        prev_lvl   = 0;			//������� ���������� �����
			double		        prev_lots  = 0;			//������� ��������� �������� �����

			int			        cur_av_lvl = 100;
			double		        base_lot  = 0;

			std::string			open_reason;			//��� �������, ����� ������� ��������

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
			//����� �������� � ������ ����
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
				if (type) { //�������
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

#pragma region ������� ��������� �������

			// ���������� "������ ����" ��� ����
			std::function<double (double a, double b)> bestPrice;

			// ���������� ���� �����������
			inline double tp(double _open_price, double _tp) {
				return _open_price - _tp * typeSign;
			}

			// ���������� ���� ���������
			inline double sl(double _open_price, double _sl) {
				return _open_price + _sl * typeSign;
			}

			// ���������� ��� ������
			inline double orderWeight(double open_price, double close_price, double _lots) {
				return (open_price - close_price) * _lots * typeSign;
			}
			inline double orderWeight(Order* order, double close_price) {
				return (order->openprice - close_price) * order->lots * typeSign;
			}

			// ���������� ������ ���� � ������ ���� �������� buy(high-low)
			inline double delta(double low_price, double high_price) {
				return (low_price - high_price) * typeSign;
			}

			// ���������� ��� �������
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

			// ���������� ���� ������� �� ��������� ������
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