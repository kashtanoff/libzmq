#pragma once

#include <vector>
#include <functional>
#include <algorithm>

#include "debug/Debug.h"
#include "Order.cpp"

namespace fxc {

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

			double		        step_peak  = 0;			//��� �������� ����
			double		        tp_peak    = 0;			//��� ��������� �����������
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

			// ���������� ������ ���� � ������ ���� ��������
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