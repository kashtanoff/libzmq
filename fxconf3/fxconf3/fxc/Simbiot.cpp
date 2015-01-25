#pragma once
#ifndef SIMBIOT_CPP
#define SIMBIOT_CPP

#include <algorithm>

#include "math.h"

#include "../stdafx.h"
#include "../Defines.h"

#include "fxc.h"
#include "Order.cpp"
#include "Parameters.cpp"
#include "Diller.cpp"

namespace fxc {

	//Структура описывающая все данные одного экземпляра советника
	class Simbiot : public Parameters
	{
		public:
#pragma region Переменные и ссылки
	
			//Переменные менеджера ордеров и общей логики
			int		index;
			int		old_index;
			int		total_count;
			int		old_count;
			Diller*	dillers[2];
			Diller*	curdil;

			Order	orders[NUM_ORDERS];
			Order	old_orders[NUM_ORDERS];

			double	base_lot;			//базовый лот
			double	equity;
			double	max_equity;
			//double	prev_indicator;
			//move_tp
			double	m_weight;
			double	m_last_weight;
			double	m_total_weight;
			int		m_index;
			int		k;					//Этап алгоритма для покупки и продажи
			bool    sorted;
			bool	showend;
			double	profits[50];
			double	p_high;
			double  p_low;
			double  p_buy;
			double  p_sell;

			//Переменные для реализации индикаторов
			double*	closes;
			double*	highs;
			double* lows;
			double	tmAvr;
			double	tmBuffer[MAX_BARS];
			double	up_ind;
			double	dn_ind;
			double  wu, wd;
			double  prev_wu, prev_wd;
			double	wuBuffer[MAX_BARS];
			double	wdBuffer[MAX_BARS];
			double	maBuffer[3];
			int		counted;
			int		bars;
			double  tmup, tmdown;
			bool	first_calc;
			//bool	prev

#pragma endregion

			Simbiot(char* _symbol)
			{
				strcpy(symbol, _symbol);
				dillers[0] = new Diller(this, 0);
				dillers[1] = new Diller(this, 1);
				dillers[0]->opposite = dillers[1];  //Обмен ссылками друг на друга
				dillers[1]->opposite = dillers[0];
				curdil = dillers[0];
				total_count = 0;
				p_high = 0;
				p_low = 1000000;
				p_buy = 0;
				p_sell = 0;
				counted = 0;


				k = 0;
				m_index = 0;
				c_index = -1;
				c_all = false;
				showend = true;

				//msg << "Simbiot OK" << msg_box;
			}

			void PostInit()
			{
				//bp = true;
				//msg << "tp_mult=" << _tp_mult << "\r\n";
				for (int i = 0; i < 50; i++)
				{
					profits[i] = _takeprofit * pow(_pips_mult, i);
					//msg << "steps[" << i << "]=" << steps[i] << ", tps = " << tps[i] << "\r\n";
				}
				*indicator = 0;
				prev_indicator = -50;
				first_calc = true;
				dillers[0]->_base_lot = _buy_lot;
				dillers[1]->_base_lot = _sell_lot;
				curdil->cur_av_lvl = _av_lvl;
				base_lot = curdil->_base_lot;
				//msg << msg_box;
				//bp = false;
				//msg << "trailing step: " << _tr_step << "\r\n";
				//msg << "chk_new min" << min_sl_tp << msg_box;
			}

			void Sort()
			{
				if (sorted) return;
				if (dillers[0]->level = dillers[0]->orders.size())
				{
					std::sort(
						dillers[0]->orders.begin(),
						dillers[0]->orders.end(),
						[](const Order* a, const Order* b) {
						return a->openprice > b->openprice;
					}
					);
					dillers[0]->first = dillers[0]->orders[0];
					dillers[0]->last = dillers[0]->orders.back();
				}
				else
				{
					dillers[0]->first = dillers[0]->last = nullptr;
				}

				if (dillers[1]->level = dillers[1]->orders.size())
				{
					std::sort(
						dillers[1]->orders.begin(),
						dillers[1]->orders.end(),
						[](const Order* a, const Order* b) {
						return a->openprice < b->openprice;
					}
					);
					dillers[1]->first = dillers[1]->orders[0];
					dillers[1]->last = dillers[1]->orders.back();
				}
				else
				{
					dillers[1]->first = dillers[1]->last = nullptr;
				}

				curdil = dillers[0];
				sorted = true;
				*count_p = dillers[0]->level;
				*(count_p + 1) = dillers[1]->level;
			}


			void calc_first_lot()
			{
				curdil->cur_av_lvl = _av_lvl;		//Восстанавливаем текущий уровень усреднения на максимум
				if (_auto_mm > 0)				//Если включен манименеджмент
				{
					base_lot = normlot(floor(equity / _mm_equ) * lot_step);
					curdil->_base_lot = base_lot;
				}
				else
					base_lot = curdil->_base_lot;

				*o_lots = base_lot;
				if (curdil->opposite->level >= _trend_lvl)	//Отрабатываем увеличение при тренде
				{
					*o_lots = max(*o_lots, base_lot * _trend_lot_mult * pow(_trend_progress, curdil->opposite->level - _trend_lvl));
				}
				else if (curdil->prev_lvl >= _repeat_lvl)		//Отрабатываем увеличение при повторе
					*o_lots = max(*o_lots, base_lot * _repeat_lot_mult * pow(_repeat_progress, curdil->prev_lvl - _repeat_lvl));
				//msg << "repeat: " << *o_lots << "\r\n";}
				if (curdil->opposite->last)
					*o_lots = max(*o_lots, curdil->opposite->last->lots * _lot_hadge_mult);		//Отрабатываем простое хэджирование
				if (_weighthadge)
					*o_lots = max(*o_lots, (_takeprofit * base_lot - curdil->opposite->basket_weight(*o_tpprice, 100)*_weighthadge) / _takeprofit);
				*o_lots = max(*o_lots, *(total_lots + curdil->opposite->type) * _basket_hadge_mult);
				//msg << "hadge(" << _hadge_mult << "): " << *o_lots << "\r\n";
				*o_lots = max(*o_lots, curdil->prev_lots * _regres_mult);		//Затухание
				//msg << "regres: " << *o_lots << msg_box;
				*o_lots = min(*o_lots, _maxlot);
			}
			bool calc_first()
			{
				//ShowInfo("calc_first", "start");
				/*if (_tr_step > 0 && curdil->delta(curdil->step_peak, curdil->mpo) < _tr_step)
				{
				curdil->step_peak = curdil->best_price(curdil->step_peak, curdil->mpo);
				ShowInfo("calc_first trailing peak", curdil->step_peak);
				ShowInfo("calc_first delta", curdil->delta(curdil->step_peak, curdil->mpo));
				return(false);
				}*/
				*o_openprice = curdil->mpo;
				if (curdil->level == 0)
					*o_tpprice = curdil->tp(*o_openprice, _takeprofit);
				else
					*o_tpprice = curdil->tp(*o_openprice, _takeprofit);
				//ShowInfo("calc_first: _takeprofit", _takeprofit);
				//ShowInfo("calc_first: tp_price",  curdil->tp(*o_openprice, _takeprofit));
				*o_type = curdil->type;

				*o_slprice = curdil->sl(*o_openprice, _stoploss);
				*intret = check_new();
				if (*intret > 0)
				{
					ShowInfo("*** calc_first check error", *intret);
					return(false);
				}
				/*MarketInfo();
				OrderInfo();
				msg << "First Order\r\n" << msg_box;*/
				*intret = 1;
				return(true);
			}
			bool calc_forward()
			{
				*o_openprice = curdil->tp(curdil->first->openprice, _step * _forward_step_mult);
				double d = curdil->delta(curdil->mpo, *o_openprice);
				/*msg << "try calc forward\r\n";
				msg << "mpo: " << mpo[type] << "\r\n";
				msg << "first_price: " << first_price[type] << "\r\n";
				msg << "openprice: " << *o_openprice << "\r\n";
				msg << "delta: " << d << msg_box;*/
				if (d > min_sl_tp)   //если можно выставить отложку
					*o_type = curdil->type + 4;
				else if (d <= 0)	//если нельзя отложку, но можно по рынку
				{
					*o_openprice = curdil->mpo;
					*o_type = curdil->type;
				}
				else				//Пока не можем выставить не отложку не по рынку
					return(false);
				calc_first_lot();
				*o_tpprice = curdil->tp(*o_openprice, _takeprofit);
				*o_slprice = curdil->sl(*o_openprice, _stoploss);
				*intret = check_new();
				if (*intret > 0)
				{
					ShowInfo("*** calc_forward check error", *intret);
					return(false);
				}
				return(true);
			}
			bool calc_next()
			{
				*o_openprice = curdil->sl(curdil->last->openprice, _step);
				double d = curdil->delta(*o_openprice, curdil->mpo);  //Расстояние до будущего ордера, если отрицательное, то проехали, надо ставить по рынку
				//ShowInfo("calc_next fixed step, d", d);
				if (d <= 0)  //Выставляем по рынку
				{
					*o_openprice = curdil->mpo;
					*o_type = curdil->type;
				}
				else   //Пока не можем выставить не отложку не по рынку
				{
					curdil->step_peak = 0;
					return(false);
				}
				*o_slprice = curdil->sl(*o_openprice, _stoploss);
				if (curdil->last->lots >= _av_lot)
				{
					if (curdil->cur_av_lvl == _av_lvl && curdil->level < _av_lvl)
						curdil->cur_av_lvl = max(1, curdil->level);
					else
						curdil->cur_av_lvl = max(1, curdil->cur_av_lvl - 1);
				}
				*o_tpprice = curdil->tp(*o_openprice, _takeprofit);
				double nextProfit = profits[curdil->level - 1];
				*o_lots = (nextProfit * curdil->_base_lot - curdil->basket_weight(*o_tpprice, curdil->cur_av_lvl)) / _takeprofit;
				if (_safe_copy)
					*o_lots = max(*o_lots, curdil->_base_lot);
				/*if (*o_lots > 0)
				{
				msg << "Try set next order - " << curdil->type << "\r\n";
				MarketInfo();
				//OrdersList();
				CurDilList(*o_tpprice);
				OrderInfo();
				msg << "Basket weight: " << curdil->basket_weight(*o_tpprice, curdil->cur_av_lvl)*10000 << ",  Calc lot: " << (nextProfit * curdil->_base_lot - curdil->basket_weight(*o_tpprice, curdil->cur_av_lvl)) / _takeprofit << ", Base lot: " << curdil->_base_lot << "\r\n";
				msg << "nextProfit: " << nextProfit << "\r\n" << msg_box;
				}*/
				*o_lots = min(*o_lots, _maxlot);
				*intret = check_new();
				if (*intret > 0)
				{
					ShowInfo("*** calc_next check error", *intret);
					return(false);
				}
				*intret = curdil->level + 1;
				return(true);
			}
			bool calc_oposite()
			{
				//ShowInfo("calc_oposite", "start");
				//if (curdil->opposite->level > _op_av_lvl && curdil->opposite->GetOrder())
				if (curdil->opposite->GetOrder() && abs(cur_order.openprice - curdil->mpo) > _step * _op_av_lvl)

				{
					//msg << "try calc oposite" << msg_box;
					double tp_price = curdil->tp(curdil->mpo, _takeprofit);
					double oplots = (_takeprofit * base_lot - curdil->opposite->order_weight(cur_order.openprice, tp_price, cur_order.lots)) / _takeprofit;
					if (oplots > _maxlot)  //Если лот превышает максимум, то обратку не усредняем
						return(false);
					*o_lots = max(base_lot, oplots); //Вместо base_lot было *o_lots незнаю почему
					*o_ticket = cur_order.ticket;
					*o_openprice = cur_order.openprice;
					*o_slprice = tp_price;
					*o_tpprice = cur_order.tpprice;
					*intret = check_mod();
					if (*intret > 0)
						return(false);
					return(true);
				}
				return(false);
			}
			bool move_tp()
			{
				//ShowInfo("move_tp", "start");
				if (curdil->last->tpprice == 0)
				{
					*o_ticket = curdil->last->ticket;
					*o_openprice = curdil->last->openprice;
					*o_tpprice = curdil->tp(*o_openprice, _takeprofit);
					*o_slprice = curdil->sl(*o_openprice, _stoploss);
					*intret = check_mod();
					if (*intret > 0)
					{
						ShowInfo("move_tp check_mod error", *intret, true);
						return(false);
					}
					//ShowInfo("move_tp", "mod order");
					k = 5;
					return(true);
				}
				if (curdil->level < 2)   //Если в рынке один ордер, то нечего и двигать
				{
					//ShowInfo("move_tp", "nothing to move");
					return(false);
				}
				if (curdil->order_weight(curdil->first->openprice, curdil->mpc, 1) > 0)
					return(false);


				double last_tp = curdil->last->tpprice;
				if (m_index == 0)
				{
					m_weight = 0.0;
					m_last_weight = curdil->order_weight(curdil->last->openprice, last_tp, curdil->last->lots);
					m_total_weight = curdil->basket_weight(last_tp);
				}
				//ShowInfo("move_tp", "m_index==0");
				if (m_index < curdil->level - 1)
				{
					if (!curdil->GetOrder(m_index))
					{
						m_index = 0;
						//ShowInfo("move_tp", "no order");
						return(false);
					}
					if (m_total_weight < 0.0)
					{
						m_weight += curdil->order_weight(cur_order.openprice, last_tp, cur_order.lots);
						if (m_last_weight + m_weight <= 0.0)
						{
							m_index = 0;
							//ShowInfo("move_tp", "cant average");
							return(false);
						}
					}
					if (abs(cur_order.tpprice - last_tp) > point)
					{
						m_index++;
						*o_ticket = cur_order.ticket;
						*o_openprice = cur_order.openprice;
						*o_tpprice = last_tp;
						*o_slprice = cur_order.slprice;
						*intret = check_mod();
						if (*intret > 0)
						{
							ShowInfo("move_tp check_mod error", *intret, true);
							return(false);
						}
						//ShowInfo("move_tp", "mod order");
						return(true);
					}
				}
				m_index = 0;
				return(false);
			}
			bool signal1()
			{

				if (_first_free && curdil->level == 0)  //Первый ордер открываем в любом случае
					return true;
				/*
				if (curdil->type)
				{
				if (*indicator > - _rsi_delta && *indicator2 < -_rsi_delta)
				return true;
				}
				else
				{
				if (*indicator < (_rsi_delta - 100) && *indicator2 > (_rsi_delta - 100))
				return true;
				}
				*/

				/*if (curdil->type)
				{
				if (*indicator >= -_rsi_delta)
				prev_indicator = max(prev_indicator, *indicator);
				if (prev_indicator >= - _rsi_delta && *indicator < -_rsi_delta2)
				{
				prev_indicator = -50;
				bp = true;
				return true;}
				}
				else
				if (*indicator <= (_rsi_delta - 100))
				prev_indicator = min(prev_indicator, *indicator);
				if (prev_indicator <= (_rsi_delta - 100) && *indicator > (_rsi_delta2 - 100))
				{
				prev_indicator = -50;
				bp = true;
				return true;}*/
				/*if (dillers[0]->mpc - *indicator > _rsi_delta)
				prev_indicator = 1;
				if (*indicator - dillers[0]->mpc > _rsi_delta)
				prev_indicator = -1;

				if (curdil->type)
				{

				if (prev_indicator > 0 && curdil->mpo < *indicator)
				{
				prev_indicator = 0;
				return true;
				}
				}
				else
				{
				if (prev_indicator < 0 && curdil->mpc > *indicator)
				{
				prev_indicator = 0;
				return true;
				}
				}*/
				/*if (curdil->type)
				{
				if (prev_indicator > 0 && *indicator < *indicator2 && *indicator > 100 - _rsi_delta2 && *indicator < 100 - _rsi_delta)
				{
				return true;
				}
				}
				else
				{
				if (prev_indicator < 0 && *indicator > *indicator2 && *indicator < _rsi_delta2 && *indicator > _rsi_delta)
				{
				return true;
				}
				}*/
				//if (prev_indicator == *indicator)
				//return false;
				/*
				if (curdil->type)
				{
				if (*indicator > _rsi_delta && *indicator2 <= _rsi_delta)
				return true;
				} else {
				if (*indicator < -_rsi_delta && *indicator2 >= -_rsi_delta)
				return true;
				}*/
				if (curdil->type)
				{
					if (*indicator > 0)
						return true;
				}
				else
				{
					if (*indicator < 0)
						return true;
				}

				return false;
			}
			bool signal()
			{
				if (_first_free && curdil->level == 0)  //Первый ордер открываем в любом случае
					return true;
				//*indicator = 0;
				//*indicator2 = 0;
				//bp = true;
				if (curdil->level >= _free_lvl)
					return true;
				//bool fract = tmBuffer[_period] < tmup && tmup > tmBuffer[1];
				//double d;
				if (curdil->type) //Продажи
				{
					//d= _deviation - max((((curdil->level)?curdil->mpo - curdil->last->openprice: 0) - _step) * _multf, 0);
					//*indicator2 = d;
					if (curdil->step_peak)
					{
						if ((_periodf3 && maBuffer[0] < maBuffer[1]) ||
							(_periodf3 == 0 && curdil->step_peak - curdil->mpo >= _rollback) ||
							(_periodf3 == 0 && _rollback == 0))
						{
							curdil->step_peak = 0;
							return true;
						}
						else
							curdil->step_peak = max(curdil->step_peak, curdil->mpo);

					}
					else if (curdil->mpo > up_ind)
					{
						//msg << "start trall sell" << msg_box;
						curdil->step_peak = curdil->mpo;
					}
					/*if (fract && !prev_indicator)// || (tmBuffer[_period] < tmup && tmup > tmBuffer[1]))
					{
					prev_indicator = fract;
					*indicator2 = 1;
					//_step = up_ind - tmBuffer[0];
					//_takeprofit = _step * 1.5;
					return true;
					}*/
					/*if (*indicator2 > 0)
					{
					_step = max(0.001, *indicator2 * 0.4);
					_takeprofit = max(0.0015, *indicator2 * 0.6);
					return true;}*/


					/*
					if (*indicator2 == 0)
					return false;
					//msg >> "ind: " >> *indicator
					_step = max(0.001, *indicator2 * 0.4);
					_takeprofit = max(0.0015, *indicator2 * 0.6);*/
				}
				else //Покупки
				{
					//d= _deviation - max((((curdil->level)?curdil->last->openprice - curdil->mpo: 0) - _step) * _multf, 0);
					//*indicator = d;
					if (curdil->step_peak)
					{
						if ((_periodf3 && maBuffer[0] > maBuffer[1]) ||
							(_periodf3 == 0 && curdil->mpo - curdil->step_peak >= _rollback) ||
							(_periodf3 == 0 && _rollback == 0))
						{
							curdil->step_peak = 0;
							return true;
						}
						else
							curdil->step_peak = min(curdil->step_peak, curdil->mpo);

					}
					else if (curdil->mpc < dn_ind)
					{
						//msg << "start trall buy" << msg_box;
						curdil->step_peak = curdil->mpo;
					}
					/*if (!fract && prev_indicator)//(tmBuffer[_period] > tmdown && tmdown < tmBuffer[1]))
					{
					prev_indicator = fract;
					*indicator = 1;
					//_step = tmBuffer[0] - dn_ind;
					//_takeprofit = _step * 1.5;
					return true;
					}*/
					/*if (*indicator >0)
					{
					_step = max(0.001, *indicator * 0.4);
					_takeprofit = max(0.0015, *indicator * 0.6);
					return true;}*/
					/*
					if (*indicator == 0)
					return false;
					_step = max(0.001, *indicator * 0.4);
					_takeprofit = max(0.0015, *indicator * 0.6);*/
				}
				//prev_indicator = fract;
				return false;
			}
			void calc_indicator()
			{
				__try
				{
					double main_avr;
					double diffup;
					double diffdn;
					//msg << "buf_len: " << _buf_len << "\r\n";
					closes[_buf_len] = dillers[0]->mpc;
					highs[_buf_len]  = max(dillers[0]->mpc, highs[_buf_len]);
					lows[_buf_len]   = min(dillers[0]->mpc, lows[_buf_len]);

					int start = min(bars - counted, _periodf2);
					if (start == _periodf2)
					{
						prev_wd = prev_wu = _delta*_delta;
					}

					//msg << "start: " << start << "\r\n";
					for (int i = start; i >= 0; i--)
					{
						main_avr = GetMAW(_period, i);
						diffup   = max(highs[_buf_len - i] - main_avr, _delta);
						diffdn   = max(main_avr - lows[_buf_len - i], _delta);
						diffup  *= diffup;
						diffdn  *= diffdn;

						wu = (prev_wu*(_periodf2 - 1) + diffup) / _periodf2;
						wd = (prev_wd*(_periodf2 - 1) + diffdn) / _periodf2;

						if (i > 0)
						{
							prev_wu = wu;
							prev_wd = wd;
						}
					}

					//msg << "end\r\n";
					counted = bars;
					up_ind = main_avr + _deviation * sqrt(wu);
					dn_ind = main_avr - _deviation * sqrt(wd);

					if (_periodf3)
					for (int i = 1; i >= 0; i--)
					{
						maBuffer[i] = GetMAW(_periodf3, i);
					}
					//*indicator2 = up_ind;
					//msg << msg_box;
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					k  = 100;
					bp = true;
					ShowInfo("calc_indicator ERROR", GetExceptionCode(), true);
				};
			}

			double GetMAW(int period, int shift)
			{
				int j, k;
				double sum = (period + 1) * closes[_buf_len - shift];
				double sumw = period + 1;
				for (j = 1, k = period; j <= period; j++, k--)
				{
					sum += k * closes[_buf_len - (shift + j)];
					sumw += k;
				}
				return sum / sumw;
			}


			void refresh_prices(double *_closes, double *_highs, double *_lows, int _bars)
			{
				__try
				{
					closes = _closes;
					highs = _highs;
					lows = _lows;
					bars = _bars;
					calc_indicator();
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					k = 100;
					bp = true;
					ShowInfo("refresh_prices ERROR", GetExceptionCode(), true);
				};

			}
			bool close_profit()
			{
				//bp = true;
				//ShowInfo("c_index=", c_index);
				//ShowInfo("weight=", curdil->opposite->basket_weight(curdil->opposite->mpc), true);
				if (c_index < 0 && curdil->opposite->basket_cost() > 0)//weight(curdil->opposite->mpc) > 0)  //Инициализация
				{
					c_index = curdil->opposite->level;

				}
				if (c_index > 0)  //Если есть что закрывать - закрываем
				{
					c_index--;
					*o_ticket = curdil->opposite->orders[c_index]->ticket;
					*o_lots = curdil->opposite->orders[c_index]->lots;
					*o_openprice = curdil->opposite->mpc;
					return true;
				}
				c_index = -1;
				return false;
			}
			bool close_profit1()
			{
				if (curdil->level > c_index && curdil->basket_weight(curdil->mpc)>0)// first->profit > 0)
				{


					//msg << "I have profit!" << msg_box;
					if (curdil->type == OP_SELL && *indicator > 0)  //Текущая операция sell
					{
						//msg << "SELL must be closed" << msg_box;
						*o_ticket = curdil->orders[c_index]->ticket;
						*o_lots = curdil->orders[c_index]->lots;
						*o_openprice = curdil->mpc;
						c_index++;
						return true;
					}
					if (curdil->type == OP_BUY && *indicator2 > 0)  //Текущая операция buy
					{
						//bp = true;
						//msg << "BUY must be closed" << msg_box;
						*o_ticket = curdil->orders[c_index]->ticket;
						*o_lots = curdil->orders[c_index]->lots;
						*o_openprice = curdil->mpc;
						c_index++;
						return true;
					}
				}
				c_index = 0;
				return false;
			}
			bool close_profit2()
			{
				//msg << "close profit, type: " << type << "\r\n";
				//msg << "index=" << c_index << "\r\n";
				if (c_index < 0)  //Инициализация
				{
					c_index = 0;
					c_weight = curdil->basket_weight(curdil->mpc);
					if (c_weight > 0.0)   //Если вес всей сетки положительный, то закрываем всю сетку
						c_all = true;
					else
					{
						c_weight = curdil->order_weight(curdil->last->openprice, curdil->mpc, curdil->last->lots - curdil->_base_lot);
						c_all = false;
					}
				}
				if (c_all && c_weight>0 && curdil->opposite->level >= _op_av_lvl)   //Обратное усреднение
				{
					//msg << "opposite average: " << count[1-type] << "\r\n";
					if (curdil->opposite->CloseByWeight())
						return(true);
					c_index = 0;
				}
				//msg << "basket weight=" << basket_weight(mpc[type]) * 10000 << "\r\n";
				//msg << "c_weight=" << c_weight * 10000 << "\r\n";
				//msg << "c_all=" << c_all << "\r\n";
				if (!curdil->GetOrder(c_index))
				{
					c_index = -1;
					//c_weight = 0.0;
					//msg << "no order" << msg_box;
					return(false);
				}
				if (c_all)  //Если усредняем всю сетку
				{
					*o_ticket = cur_order.ticket;
					*o_openprice = curdil->mpc;
					*o_lots = cur_order.lots;
					c_index++;
					if (c_index >= curdil->level)
					{
						c_index = -1;
						k = 100;
					}
					//msg << "close_all" << msg_box;
					return(true);
				}
				double weight = curdil->order_weight(cur_order.openprice, curdil->mpc, cur_order.lots);
				//msg << "weight=" << weight * 10000 << "\r\n";
				if (c_weight + weight < 0.0)  //Если нельзя усреднить целый ордер
				{
					//msg << "partial close\r\n";
					//msg << "с_weight=" << c_weight * 10000 << "\r\n";
					//msg << "order weight=" << weight * 10000 << "\r\n";
					double min_weight = curdil->order_weight(cur_order.openprice, curdil->mpc, lot_min);
					//msg << "min weight=" << min_weight * 10000 << "\r\n";
					if (c_weight + min_weight > 0.0)  //Если можно усреднить хотябы минимальный лот
					{
						*o_ticket = cur_order.ticket;
						*o_openprice = curdil->mpc;
						*o_lots = floor(c_weight / abs(min_weight)) * lot_step;
						c_weight += curdil->order_weight(cur_order.openprice, curdil->mpc, *o_lots);
						//msg << "full lots=" << cur_order.lots << "\r\n";
						//msg << "part lots=" << *o_lots << "\r\n";
						//msg << "last c_weight=" << c_weight * 10000 << "\r\n" << msg_box;
						return(true);
					}
					//Усреднили все что можно, закрываем верхний прибыльный ордер
					c_index = -1;
					*o_ticket = curdil->last->ticket;
					*o_openprice = curdil->mpc;
					*o_lots = curdil->last->lots;
					k = 100;
					return(true);
				}

				c_weight += weight;
				//msg << "end weight2=" << c_weight * 10000 << "\r\n";
				//msg << "close_one" << msg_box;
				c_index++;
				if (c_index >= curdil->level)
				{
					c_index = -1;
					k = 100;
				}
				*o_ticket = cur_order.ticket;
				*o_openprice = curdil->mpc;
				*o_lots = cur_order.lots;
				return(true);
			}


			//Нормализует цену
			double norm(double value)
			{
				return(floor(value / point + 0.5) * point);
			}
			//Нормализует лот
			double normlot(double value)
			{
				int steps = int(ceil(value / lot_step));
				value = steps * lot_step;
				value = max(value, lot_min);
				return min(value, lot_max);
			}
			//Проверка уровня стоплосса или тейкпрофита
			bool check_sl(int _type, double low_price, double high_price)   //Понятия high low для покупки
			{
				return(!(low_price && high_price && dillers[_type % 2]->delta(low_price, high_price) < min_sl_tp));
			}
			//Проверка уровня заморозки
			bool check_freeze(double pr1, double pr2)
			{
				return(abs(pr1 - pr2) > freeze);
			}
			//Проверка на правильность параметров при открытии ордера
			int check_new()
			{
				int t = *o_type % 2;
				if (*o_type < 2 && //Для немедленного исполнения
					(!check_sl(t, *o_slprice, dillers[t]->mpc) ||
					!check_sl(t, dillers[t]->mpc, *o_tpprice)))
				{
					ShowInfo("chk_new tp", *o_tpprice);
					ShowInfo("chk_new sl", *o_tpprice);
					ShowInfo("chk_new min", min_sl_tp);
					return(201);
				}
				if (*o_type > 1 &&  //Для отложенных ордеров
					(!check_sl(t, *o_openprice, *o_tpprice) ||
					!check_sl(t, *o_slprice, *o_openprice)))
					return(201);
				if ((*o_type == OP_BUYLIMIT || *o_type == OP_SELLLIMIT) &&
					!check_sl(t, *o_openprice, dillers[t]->mpo))
					return(202);
				if ((*o_type == OP_BUYSTOP || *o_type == OP_SELLSTOP) &&
					!check_sl(t, dillers[t]->mpo, *o_openprice))
					return(202);
				*o_lots = normlot(*o_lots);
				*o_openprice = norm(*o_openprice);
				*o_tpprice = norm(*o_tpprice);
				*o_slprice = norm(*o_slprice);
				return(0);
			}
			//Проверка на правильность параметров при модификации ордера
			int check_mod()
		{
			if (get_order(*o_ticket) == 0)
				return(200);
			*o_openprice = norm(*o_openprice);
			*o_tpprice = norm(*o_tpprice);
			*o_slprice = norm(*o_slprice);
			int   _type = cur_order.type;
			double _mpc = dillers[_type]->mpc;
			double _mpo = dillers[_type]->mpo;

			if (_type < 2 &&   //Для открытых ордеров
				(!check_freeze(cur_order.slprice, _mpc) ||
				!check_freeze(cur_order.tpprice, _mpc) ||
				!check_sl(_type, *o_slprice, _mpc) ||
				!check_sl(_type, _mpc, *o_tpprice)))
				return(201);

			if (_type > 1 &&  //Для отложенных ордеров
				(!check_freeze(cur_order.openprice, _mpo) ||
				!check_sl(_type, *o_slprice, *o_openprice) ||
				!check_sl(_type, *o_openprice, *o_tpprice)))
				return(201);

			if ((_type == OP_BUYLIMIT || _type == OP_SELLLIMIT) &&
				!check_sl(_type, cur_order.openprice, _mpo))
				return(202);

			if ((_type == OP_BUYSTOP || _type == OP_SELLSTOP) &&
				!check_sl(_type, _mpo, cur_order.openprice))
				return(202);

			return(0);
		}


#pragma region Секция функционала менеджера ордеров

			//3.1 Инициализация цикла обновления ордеров, поскольку цикл получаеться рваным (каждая итерация вызывается из MQL), нельзя пользоватья результатами в MQL программе до завершения цикла
			void refresh_init(double ask, double bid, double equity_)   //+
			{
				__try
				{
					sorted = false;
					*o_lots = 0;
					equity = equity_;
					dillers[0]->mpo = dillers[1]->mpc = ask;
					dillers[1]->mpo = dillers[0]->mpc = bid;
					//mpo[0] = mpo[2] = mpo[4] = mpc[1] = mpc[3] = mpc[5] = ask;
					//mpo[1] = mpo[3] = mpo[5] = mpc[0] = mpc[2] = mpc[4] = bid;
					//max_equity = max(max_equity, equity);
					//*max_dd = max(max_equity - equity, *max_dd);
					index = 0;  //Обнуляем общий счетчик ордеров
					//ShowInfo("refresh_init", "bp1");
					//prev_indicator = (*indicator==10)? prev_indicator : *indicator;
					//*indicator = 10;
					if (!is_optimization)   //Если не оптимизация
					{
						old_index = 0;
						memcpy(old_orders, orders, sizeof(orders));  //список текущих ордеров, делаем старым
						old_count = total_count;
					}
					//ShowInfo("refresh_init", "bp2");
					dillers[0]->ResetTick();    //Сбрасываем сортировочные показатели по типам
					dillers[1]->ResetTick();
					k = 0;			//С новым тиком сбрасываем итератор операции
					c_index = -1;
					if (!_new_bar)
						calc_indicator();
					//msg << "refresh init done" << msg_box;
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					k = 100;
					bp = true;
					msg << "refresh_init ERROR: " << GetExceptionCode() << msg_box;
				};
			}
			//3.1 Добавляет новый ордер в цикле скана ордеров, в будущем возвращает код изменения
			int refresh_order(int _ticket, int _type, double _lots, double _openprice, double _tp, double _sl, double _profit = 0)
			{
				__try
				{
					//msg << "refresh start" << msg_box;
					orders[index].ticket = _ticket;
					orders[index].type = _type;
					orders[index].lots = _lots;
					orders[index].openprice = _openprice;
					orders[index].tpprice = _tp;
					orders[index].slprice = _sl;
					//orders[index].checked = false;
					if (_type<2)
					{
						orders[index].profit = _profit;
						*(open_dd + _type) += _profit;
						*(total_lots + _type) += _lots;
						dillers[_type]->orders.push_back(&orders[index]);
					}
					else if (_type < 4)
						dillers[_type % 2]->ord_limit = &orders[index];
					else
						dillers[_type % 2]->ord_stop = &orders[index];

					total_count = ++index;
					//msg << "refresh order done " << index << msg_box;
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					k = 100;
					bp = true;
					msg << "refresh_order ERROR: " << GetExceptionCode() << msg_box;
				};

				return(0);
			}
			//Выдает тикеты закрытых ордеров, пока они есть
			int getclosed()
			{
				bool _flag;
				while (old_index < old_count)
				{
					if (old_orders[old_index].type > OP_SELL)   //Пропускаем не рыночные ордера
					{
						old_index++;
						continue;
					}
					_flag = false;
					for (int i = 0; i<total_count; i++)
					{
						if (old_orders[old_index].ticket == orders[i].ticket)
						{
							_flag = true;
							break;
						}
					}
					old_index++;
					if (!_flag)
						return(old_orders[old_index - 1].ticket);
				}
				return(0);
			}
			//выдает ордер из менеджера по тикету
			bool get_order(int _ticket)
			{
				for (int i = 0; i<total_count; i++)
				if (orders[i].ticket == _ticket)
				{
					cur_order = orders[i];
					return(true);
				}
				return(false);  //Не нашли
			}

			//		ShowInfo("calc_first_lot", "BreakPoint 1", true);
			//} catch(std::exception e) {ShowInfo("ERROR: calc_first_lot", e.what(), true);};
			template <class T> void ShowInfo(char text[], T value, bool show = false)
			{
		#if DEBUG 
				//bp = true;
				if (!is_visual)
					return;
				if (showend)
				{
					msg << "Ask: " << dillers[0]->mpo << "   Bid: " << dillers[1]->mpo << "\r\n";
					LPCSTR s = (curdil->type) ? " Sell" : "  Buy";
					msg << "Type: " << curdil->type << s << " sorted=" << sorted << "\r\n";
					msg << "Count Buy: " << dillers[0]->level << " (" << dillers[0]->orders.size() << "), Sell: " << dillers[1]->level << "\r\n";
					msg << "k=" << k << " total: " << total_count << "\r\n";
					msg << "o_type=" << *o_type << ", o_lot=" << *o_lots << "\r\n";
					msg << "o_price=" << *o_openprice << ", o_tp=" << *o_tpprice << "\r\n";
					showend = false;
				}

				msg << text << ": " << value << "\r\n";
				if (show)
				{
					showend = true;
					msg << msg_box;
					//bp = false;
				}
		#endif
			}
			void MarketInfo()
			{
				msg << "Ask: " << dillers[0]->mpo << "   Bid: " << dillers[1]->mpo << "\r\n";
			}
			void OrderInfo()
			{
				msg << "o_type=" << *o_type << ", o_lot=" << *o_lots << "\r\n";
				msg << "o_price=" << *o_openprice << ", o_tp=" << *o_tpprice << "\r\n";
			}
			void OrdersList()
			{
				msg << "buy orders - " << dillers[0]->level << " (" << dillers[0]->basket_weight(dillers[0]->mpc) * 10000 << ")\r\n";
				if (dillers[0]->level)
				for (int i = 0; i<dillers[0]->level; i++)
					msg << "#" << i << " - op:" << dillers[0]->orders[i]->openprice << ", tp:" << dillers[0]->orders[i]->tpprice << ", lot:" << dillers[0]->orders[i]->lots << " (" << dillers[0]->order_weight(dillers[0]->orders[i]->openprice, dillers[0]->mpc, dillers[0]->orders[i]->lots) * 10000 << ")\r\n";
				msg << "sell orders - " << dillers[1]->level << " (" << dillers[1]->basket_weight(dillers[1]->mpc) * 10000 << ")\r\n";
				if (dillers[1]->level)
				for (int i = 0; i<dillers[1]->level; i++)
					msg << "#" << i << " - op:" << dillers[1]->orders[i]->openprice << ", tp:" << dillers[1]->orders[i]->tpprice << ", lots:" << dillers[1]->orders[i]->lots << " (" << dillers[1]->order_weight(dillers[1]->orders[i]->openprice, dillers[1]->mpc, dillers[1]->orders[i]->lots) * 10000 << ")\r\n";
			}
			void CurDilList(double tp)
			{
				msg << "type: " << curdil->type << " (" << curdil->basket_weight(tp) * 10000 << ")\r\n";
				if (curdil->level)
				for (int i = 0; i<curdil->level; i++)
					msg << "#" << i << " - op:" << curdil->orders[i]->openprice << ", tp:" << curdil->orders[i]->tpprice << ", lot:" << curdil->orders[i]->lots << " (" << curdil->order_weight(curdil->orders[i]->openprice, tp, curdil->orders[i]->lots) * 10000 << ")\r\n";
			}

#pragma endregion


			int getjob()
			{
				__try
				{
					//ShowInfo("======== get_job", "start ========");
					while (true)		//Чтобы лишний раз не выходить в MQL
					{
						//ShowInfo("----- getjob work", k);
						switch (k)
						{
						case 0: //Старт нового тика
							Sort();
							if (!curdil->level)		//Если нет ордеров в рынке
							{
								if (!_stop_new[curdil->type] && (signal() || curdil->opposite->level >= _forward_lvl))	//Если не запрещено открытие новой сетки и есть сигнал
								{  //Открываем первый ордер
									calc_first_lot();
									k = 1;
									if (calc_oposite())		//Отрабатываем обратное усреднение
										return(2);
									break;
								}
								k = 2; break;
							}
							else if (!_stop_avr[curdil->type] && curdil->level < _max_grid_lvl)   //Если есть базовый ордер и разрешено усреднять и максимальный уровень не достигнут
							{
								k = 4;
								if (curdil->opposite->level >= _forward_lvl && !curdil->ord_stop && calc_forward())   //Если разрешены форварды и его можно поставить
									return(1);
								break;   //Наращивание ступеней сетки
							}
							else  //Если есть ордера, но не разрешено усреднять -> удалить отложки
								k = 3;
							break;
						case 1: //Открывает первый ордер
							if (_opp_close && close_profit())
								return(4);
							k = 2;
							if (calc_first())
							{
								//ShowInfo("neworder first", *o_lots);
								return(1);
							}		//Открыть первый ордер
							break;
						case 2: //удаление противоположного стоп ордера
							k = 3;
							if (curdil->opposite->ord_stop)
							{
								*o_ticket = curdil->opposite->ord_stop->ticket;
								//ShowInfo("delorder stop opposite", *o_ticket);
								return(3);  //Удалить стоп ордер
							}
							break;
						case 3:	//удаление своего стоп ордера
							k = 5;
							if (curdil->ord_stop)
							{
								*o_ticket = curdil->ord_stop->ticket;
								//ShowInfo("delorder stop", *o_ticket);
								return(3);	//Удалить стоп ордер
							}
							break;
						case 4:  //двигает тейкпрофиты, открывает последующие ордера
							if (move_tp())	//Если есть что двигать, двигаем
								return(2);
							k = 5;
							if (signal() && calc_next()) //Если можно выставить отложку или по рынку, выставляем
							{
								k = 6;
								return(1);
							}
							break;
						case 5:

							k = 100;
							break;
						case 6:
							if (_opp_close == 2 && close_profit())
								return(4);
							k = 100;

						case 100: //Завершение работы, обработка продаж или отработка таймаутов
							k = 0;  //Указатель этапов начало
							//ShowInfo("k100 start", "");
							if (curdil->type == 0)
							{
								curdil = dillers[1];
								//ShowInfo("======== Start Sell ========", "");
								break;
							}	//Все сначала только для продажи
							curdil = dillers[0];
							//ShowInfo("======== get_job", "end ========", true);
							bp = false;
							prev_indicator = *indicator;//(*indicator > *indicator2)? 1: -1;
							return(0);	//Отрабатываем таймауты и завершаем алгоритм
						}

					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					k = 0;
					curdil = dillers[0];
					bp = true;
					ShowInfo("get_job ERROR", GetExceptionCode(), true);
				};
				return(0);
			}

			//Деструктор, надо очистить то, что не удалиться на автомате
			/*~Simbiot()
			{
				delete dillers[0];
				delete dillers[1];
			}*/
	};

}

#endif