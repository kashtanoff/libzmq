#pragma once

#include "DefaultStrategy.cpp"

#define STEP_NEW_TICK        0
#define STEP_OPEN_ORDER      1
#define STEP_DELETE_OPPOSITE 2
#define STEP_DELETE_SELF     3
#define STEP_MOVE            4
#define STEP_PRE_FINALIZE    6
#define STEP_FINALIZE        100

namespace fxc {

namespace strategy {

	class OldStrategy : public DefaultStrategy {

		public:

			OldStrategy(char* _symbol) : DefaultStrategy(_symbol) {}

			const long atick = 113116;

			virtual int getJob() {
#if TRACE_STRATEGY
				if (*ext_ticker == atick)
				fxc::msg << "-> OldStrategy::getJob()\r\n" << fxc::msg_box;
#endif
				__try
				{
					while (true) { //����� ������ ��� �� �������� � MQL
#if TRACE_STRATEGY
						if (*ext_ticker == atick)
							fxc::msg << "-> OldStrategy::getJob() [" << __step << "]\r\n" << fxc::msg_box;
#endif

						switch (__step) {
							case STEP_NEW_TICK: //����� ������ ����
			#pragma region STEP_NEW_TICK
								sortOrders();
								if (!curdil->level) { //���� ��� ������� � �����
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [0.1]\r\n" << fxc::msg_box;
#endif

									if (!input_stop_new[curdil->type] && (signal() || curdil->opposite->level >= input_forward_lvl)) { //���� �� ��������� �������� ����� ����� � ���� ������, ��������� ������ �����
#if TRACE_STRATEGY
										if (*ext_ticker == atick)
											fxc::msg << "-> OldStrategy::getJob() [0.1.1]\r\n" << fxc::msg_box;
#endif

										calc_first_lot();
										__step = STEP_OPEN_ORDER;

										if (calc_oposite()) { //������������ �������� ����������
#if TRACE_STRATEGY
											if (*ext_ticker == atick)
												fxc::msg << "-> OldStrategy::getJob() [0.1.1.1]\r\n" << fxc::msg_box;
#endif

											return JOB_MODIFY;
										}

										break;
									}

#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [0.1.2]\r\n" << fxc::msg_box;
#endif
									__step = STEP_DELETE_OPPOSITE;
									break;
								}
								else if (!input_stop_avr[curdil->type] && curdil->level < input_max_grid_lvl) { //���� ���� ������� ����� � ��������� ��������� � ������������ ������� �� ���������
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [0.2]\r\n" << fxc::msg_box;
#endif

									__step = STEP_MOVE;

									if (curdil->opposite->level >= input_forward_lvl && !curdil->ord_stop && calc_forward()) { //���� ��������� �������� � ��� ����� ���������
#if TRACE_STRATEGY
										if (*ext_ticker == atick)
											fxc::msg << "-> OldStrategy::getJob() [0.2.1]\r\n" << fxc::msg_box;
#endif

										return JOB_CREATE;
									}

									break; //����������� �������� �����
								}
								else { //���� ���� ������, �� �� ��������� ��������� -> ������� �������
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [0.3]\r\n" << fxc::msg_box;
#endif
									__step = STEP_DELETE_SELF;
								}
								break;
			#pragma endregion
							case STEP_OPEN_ORDER: //��������� ������ �����
			#pragma region STEP_OPEN_ORDER
								if (input_opp_close && close_profit()) {
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [1.1]\r\n" << fxc::msg_box;
#endif

									return JOB_CLOSE;
								}

								__step = STEP_DELETE_OPPOSITE;
								if (calc_first()) {
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [1.2]\r\n" << fxc::msg_box;
#endif
									return JOB_CREATE; //������� ������ �����
								}
								break;
			#pragma endregion
							case STEP_DELETE_OPPOSITE: //�������� ���������������� ���� ������
			#pragma region STEP_DELETE_OPPOSITE
								__step = STEP_DELETE_SELF;
								if (curdil->opposite->ord_stop) {
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [2.1]\r\n" << fxc::msg_box;
#endif

									*ext_o_ticket = curdil->opposite->ord_stop->ticket;
									return JOB_DELETE; //������� ���� �����
								}
								break;
			#pragma endregion
							case STEP_DELETE_SELF:	//�������� ������ ���� ������
			#pragma region STEP_DELETE_SELF
								__step = STEP_FINALIZE;
								if (curdil->ord_stop) {
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [3.1]\r\n" << fxc::msg_box;
#endif

									*ext_o_ticket = curdil->ord_stop->ticket;
									return JOB_DELETE; //������� ���� �����
								}
								break;
			#pragma endregion
							case STEP_MOVE: //������� �����������, ��������� ����������� ������
			#pragma region STEP_MOVE
								if (move_tp()) { //���� ���� ��� �������, �������
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [4.1]\r\n" << fxc::msg_box;
#endif

									return JOB_MODIFY;
								}

								__step = STEP_FINALIZE;
								if (signal() && calc_next()) { //���� ����� ��������� ������� ��� �� �����, ����������
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [4.2]\r\n" << fxc::msg_box;
#endif

									__step = STEP_PRE_FINALIZE;
									return JOB_CREATE;
								}
								break;
			#pragma endregion
							case STEP_PRE_FINALIZE:
			#pragma region STEP_PRE_FINALIZE
								if (input_opp_close == 2 && close_profit()) {
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [6.1]\r\n" << fxc::msg_box;
#endif

									return JOB_CLOSE;
								}

								__step = STEP_FINALIZE;
								break;
			#pragma endregion
							case STEP_FINALIZE: //���������� ������, ��������� ������ ��� ��������� ���������
			#pragma region STEP_FINALIZE
								__step = STEP_NEW_TICK;  //��������� ������ ������
								if (curdil->type == 0) {
#if TRACE_STRATEGY
									if (*ext_ticker == atick)
										fxc::msg << "-> OldStrategy::getJob() [100.1]\r\n" << fxc::msg_box;
#endif

									curdil = dillers[1];
									break;
								} //��� ������� ������ ��� �������

#if TRACE_STRATEGY
								if (*ext_ticker == atick)
									fxc::msg << "-> OldStrategy::getJob() [100.2]\r\n" << fxc::msg_box;
#endif

								curdil = dillers[0];
								prev_indicator = *ext_indicator;
								return JOB_EXIT; //������������ �������� � ��������� ��������
			#pragma endregion
						}

					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					__step = STEP_NEW_TICK;
					curdil = dillers[0];
					ShowInfo("get_job ERROR", GetExceptionCode(), true);
				}

				return JOB_EXIT;
			}

			virtual void refresh_init(double ask, double bid, double equity_) {
				DefaultStrategy::refresh_init(ask, bid, equity_);
				__step = STEP_NEW_TICK;
			}

		private:

			int __step;

	};
}

}