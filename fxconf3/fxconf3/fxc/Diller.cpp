#pragma once

#include <vector>

#include "Order.cpp"
#include "Parameters.cpp"

namespace fxc {

	typedef std::vector<Order*> OrderList;

	class Diller
	{
		public:
			Diller*     opposite;
			Parameters*	params;
			int			type;
			OrderList	orders;
			Order*      ord_limit;
			Order*      ord_stop;
			double		mpo;
			double		mpc;
			int			level;
			double		step_peak;		//пик трейлинг шага
			double		tp_peak;			//пик трейлинга тейкпрофита
			Order*      last;
			Order*      first;
			//int			_stop_new;		//50, 51 Остановить открытие новой сетки
			//int			_stop_avr;		//52, 53 Остановить открытие новой ступени
			int			prev_lvl;		//уровень предыдущей сетки
			double		prev_lots;		//Размеры последних закрытых лотов
			bool		sorted;
			int			cur_av_lvl;
			int			index;
			double		_base_lot;


			Diller(Parameters* _params, int _type) {
				params = _params;
				type   = _type;
				Reset();
			}

			void Reset() {
				tp_peak    = 0;
				step_peak  = 0;
				orders.clear();
				prev_lvl   = 0;
				level      = 0;
				sorted     = false;
				last       = nullptr;
				first      = nullptr;
				index      = 0;
				cur_av_lvl = 100;
				_base_lot  = 0;
			}

			void ResetTick() {
				prev_lvl  = level ? level : prev_lvl;
				prev_lots = last ? last->lots : prev_lots;

				level = 0;
				orders.clear();

				*(params->ext_open_dd    + type) = 0.0;
				*(params->ext_total_lots + type) = 0.0;
				ord_limit = nullptr;
				ord_stop  = nullptr;
			}

			void AddOrder(Order* order) {
				orders.push_back(order);
			}

			bool GetOrder(int _num = 0) {
				if (_num >= level) return false;
				params->cur_order = *orders[_num];
				return true;
			}

			//bool CloseByWeight()
			//{
			//	//msg << "opposite index: " << c_index << "\r\n";
			//	if (params->c_index >= level)  //Если ордеров больше нет, то усреднение закончено
			//	{
			//		//c_index = -1;
			//		//c_weight = 0.0;
			//		params->c_weight = -1;
			//		//msg << "no order opp" << "\r\n";
			//		return(false);
			//	}
			//	double weight = order_weight(params->cur_order.openprice, mpc, params->cur_order.lots);
			//	if (params->c_weight + weight < 0.0)  //Если нельзя усреднить целый ордер
			//	{
			//		//msg << "opp_partial close\r\n";
			//		//msg << "opp_с_weight=" << c_weight * 10000 << "\r\n";
			//		//msg << "opp_order weight=" << weight * 10000 << "\r\n";
			//		double min_weight = order_weight(params->cur_order.openprice, mpc, params->input_lot_min);
			//		//msg << "opp_min weight=" << min_weight * 10000 << "\r\n";
			//		if (params->c_weight + min_weight > 0.0)  //Если можно усреднить хотябы минимальный лот
			//		{
			//			*(params->ext_o_ticket) = params->cur_order.ticket;
			//			*(params->ext_o_openprice) = mpc;
			//			*(params->ext_o_lots) = floor(params->c_weight / abs(min_weight)) * params->input_lot_step;
			//			params->c_weight += order_weight(params->cur_order.openprice, mpc, *params->ext_o_lots);
			//			//msg << "opp_full lots=" << cur_order.lots << "\r\n";
			//			//msg << "opp_part lots=" << *o_lots << "\r\n";
			//			//msg << "opp_last c_weight=" << c_weight * 10000 << "\r\n";
			//			return(true);
			//		}
			//		//Усреднили все что можно
			//		//c_index = -1;
			//		params->c_weight = -1;
			//		return(false);
			//	}
			//	params->c_weight += weight;
			//	*(params->ext_o_ticket) = params->cur_order.ticket;
			//	*(params->ext_o_openprice) = mpc;
			//	*(params->ext_o_lots) = params->cur_order.lots;
			//	//msg << "opp_end weight2=" << c_weight * 10000 << "\r\n";
			//	//msg << "opp_close_one" << "\r\n";
			//	params->c_index++;
			//
			//	return(true);
			//}

			double BasketCost() {
				double res;
				
				for (int i = 0; i < level; i++)
					res += 1;

				return res;
			}

#pragma region Простые сервисные функции

			//"Лучшая цена" для типа
			double best_price(double a, double b) {
				return type ? fmax(a, b) : fmin(a, b);
			}

			//Расчитывает цену тейкпрофита
			double tp(double _open_price, double _tp) {
				return type ? _open_price - _tp : _open_price + _tp;
			}

			//Расчитывает цену стоплосса
			double sl(double _open_price, double _sl) {
				return type ? _open_price + _sl : _open_price - _sl;
			}

			//Расчет веса ордера 
			double order_weight(double open_price, double close_price, double _lots) {
				return (type ? open_price - close_price : close_price - open_price) * _lots;
			}

			//Выдает дельту цены с учетом типа операции
			double delta(double low_price, double high_price) {
				return type ? low_price - high_price : high_price - low_price;
			}

			//Расчитывает вес корзины
			double basket_weight(double _close_price, int _av_lvl = 100) {
				double weight = 0.0;
				for (int i = 0; i < level; i++) {
					weight += order_weight(orders[i]->openprice, _close_price, orders[i]->lots);
					if (i + 1 >= _av_lvl) {
						return weight;
					}
				}
				return weight;
			}

			//Цена корзины на настоящий момент
			double basket_cost() {
				double cost = 0.0;
				for (int i = 0; i < level; i++) {
					cost += orders[i]->profit;
				}
				return cost;
			}

#pragma endregion

	};

}