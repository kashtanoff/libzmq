#pragma once

#include "vector"

#include "debug/Debug.h"
#include "fxc.h"
#include "../MqlUtils.cpp"
#include "OrdersManager.cpp"


namespace fxc {
	
	#pragma pack(push,1)
	struct TradeAction {
		double    lots;      // 8 bytes
		double    openprice; // 8 bytes
		double    tpprice;   // 8 bytes
		double    slprice;   // 8 bytes
		int       ticket;    // 4 bytes
		int       type;      // 4 bytes
		int       intret;    // 4 bytes
		int       actionId;  // 4 bytes
		MqlString comment;   // 12 bytes
	};
	#pragma pack(pop,1)

	class TradeManager : public OrdersManager {

		public:

			bool is_visual = false;
			
			void mapActions(void* arrayPtr, int length) {
				auto ptr = (fxc::TradeAction*) arrayPtr;

				for (auto i = 0; i < length; ++i) {
					ext_tradeActions.push_back(ptr + i);
				}

				//fxc::msg << "array  mql::[0x" << arrayPtr << "][" << length << "]\r\n" << fxc::msg_box;
				//fxc::msg << "vector [" << ext_tradeActions.size() << "] / [" << ext_tradeActions.capacity() << "]\r\n" << fxc::msg_box;
				//
				//for (auto i = 0; i < length; ++i) {
				//	//ext_tradeActions.at(i)->lots      = i + 0; // 8 bytes
				//	//ext_tradeActions.at(i)->openprice = i + 1; // 8 bytes
				//	//ext_tradeActions.at(i)->tpprice   = i + 2; // 8 bytes
				//	//ext_tradeActions.at(i)->slprice   = i + 3; // 8 bytes
				//	//ext_tradeActions.at(i)->ticket    = i + 4; // 4 bytes
				//	//ext_tradeActions.at(i)->type      = i + 5; // 4 bytes
				//	//ext_tradeActions.at(i)->intret    = i + 6; // 4 bytes
				//
				//	std::stringstream ss;
				//	ss << "~> cmnt[" << i*i << "]";
				//
				//	Mql::write2str(&vec->at(i)->comment, ss.str().c_str());
				//
				//	fxc::msg 
				//		<< "vector[" << i << "] " 
				//		<< "[0x" << ext_tradeActions.at(i) << "] "
				//		//<< vec->at(i)->o_lots << ", "
				//		//<< vec->at(i)->o_openprice << ", "
				//		//<< vec->at(i)->o_tpprice << ", "
				//		//<< vec->at(i)->o_slprice << ", "
				//		//<< vec->at(i)->o_ticket << ", "
				//		//<< vec->at(i)->o_type << ", "
				//		//<< vec->at(i)->intret << ", "
				//		<< "["
				//		<< (ptr + i)->comment.size << ", "
				//		<< (ptr + i)->comment.buffer
				//		<< "] => "
				//		<< "["
				//		<< ext_tradeActions.at(i)->comment.size << ", "
				//		<< ext_tradeActions.at(i)->comment.buffer
				//		<< "]"
				//		<< " : \"" << Mql::str2chars(&ext_tradeActions.at(i)->comment) << "\""
				//		<< "\r\n" << fxc::msg_box;
				//}
			}
			
			//�������� ������ ��������
			inline void resetActionManager() {
				actionsLen = 0;
			}

		protected:
			
			//���������� ���������� ����������� ��������
			inline int getActionsStackSize() {
				return actionsLen;
			}

			inline void createOrder(int type, double lots, double openprice, double slprice, double tpprice, std::string comment = "") {
				MARK_FUNC_IN
				if (actionsLen + 1 >= ext_tradeActions.size()) {
					fxc::msg << " -> CreateOrder(): actions limit exceeded\r\n" << fxc::msg_box;
					return;
				}
				auto action = ext_tradeActions[actionsLen++];

#if DEBUG
				action->ticket = 0;
				action->intret = 0;
#endif
				action->type      = type;
				action->lots      = lots;
				action->openprice = openprice;
				action->slprice   = slprice;
				action->tpprice   = tpprice;
				action->actionId  = JOB_CREATE;
				Mql::write2str(&action->comment, comment.c_str());
				MARK_FUNC_OUT
			}

			inline void modOrder(int ticket, double openprice, double slprice, double tpprice) {
				MARK_FUNC_IN
				if (actionsLen + 1 >= ext_tradeActions.size()) {
					fxc::msg << " -> ModOrder(): actions limit exceeded\r\n" << fxc::msg_box;
				}
				auto action = ext_tradeActions[actionsLen++];

#if DEBUG
				action->type   = 0;
				action->lots   = 0;
				action->intret = 0;
#endif
				action->ticket    = ticket;
				action->openprice = openprice;
				action->slprice   = slprice;
				action->tpprice   = tpprice;
				action->actionId  = JOB_MODIFY;
				MARK_FUNC_OUT
			}

			inline void deleteOrder(int ticket) {
				MARK_FUNC_IN
				if (actionsLen + 1 >= ext_tradeActions.size()) {
					fxc::msg << " -> DeleteOrder(): actions limit exceeded\r\n" << fxc::msg_box;
				}
				auto action = ext_tradeActions[actionsLen++];

#if DEBUG
				action->type      = 0;
				action->lots      = 0;
				action->openprice = 0;
				action->slprice   = 0;
				action->tpprice   = 0;
				action->intret    = 0;
#endif
				action->ticket = ticket;
				action->actionId = JOB_DELETE;
				MARK_FUNC_OUT
			}

			inline void closeOrder(int ticket, double lots, double openprice) {
				MARK_FUNC_IN
				if (actionsLen + 1 >= ext_tradeActions.size()) {
					fxc::msg << " -> CloseOrder(): actions limit exceeded\r\n" << fxc::msg_box;
					return;
				}
				auto action = ext_tradeActions[actionsLen++];

#if DEBUG
				action->type    = 0;
				action->slprice = 0;
				action->tpprice = 0;
				action->intret  = 0;
#endif
				action->ticket    = ticket;
				action->lots      = lots;
				action->openprice = openprice;
				action->actionId  = JOB_CLOSE;
				MARK_FUNC_OUT
			}

			inline void showValue(int key, std::string value) {
				MARK_FUNC_IN
				if (!is_visual) {
					return;
				}
				if (actionsLen + 1 >= ext_tradeActions.size()) {
					fxc::msg << " -> showValue(): actions limit exceeded\r\n" << fxc::msg_box;
					return;
				}
				auto action = ext_tradeActions[actionsLen++];

				action->ticket    = key;
				action->intret    = 0;
				action->type      = SHOW_STR_VALUE;
				action->lots      = 0;
				action->openprice = 0;
				action->slprice   = 0;
				action->tpprice   = 0;
				action->actionId  = JOB_SHOW_VALUE;
				Mql::write2str(&action->comment, value.c_str());

				//msg << "-> c_str: \"" << value.c_str() << "\" => \"" << Mql::str2chars(&action->comment) << "\" [" << action->comment.size << "] [0x" << action->comment.buffer << "]\r\n" << msg_box;
				MARK_FUNC_OUT
			}
			
			inline void showValue(int key, std::string label, int value) {
				MARK_FUNC_IN
				if (!is_visual) {
					return;
				}
				if (actionsLen + 1 >= ext_tradeActions.size()) {
					fxc::msg << " -> showValue(): actions limit exceeded\r\n" << fxc::msg_box;
					return;
				}
				auto action = ext_tradeActions[actionsLen++];

				action->ticket    = key;
				action->intret    = value;
				action->type      = SHOW_INT_VALUE;
				action->lots      = 0;
				action->openprice = 0;
				action->slprice   = 0;
				action->tpprice   = 0;
				action->actionId  = JOB_SHOW_VALUE;
				Mql::write2str(&action->comment, label.c_str());

				MARK_FUNC_OUT
			}
			
			inline void showValue(int key, std::string label, double value, int digits=5) {
				MARK_FUNC_IN
				if (!is_visual) {
					return;
				}
				if (actionsLen + 1 >= ext_tradeActions.size()) {
					fxc::msg << " -> showValue(): actions limit exceeded\r\n" << fxc::msg_box;
					return;
				}
				auto action = ext_tradeActions[actionsLen++];

				action->ticket    = key;
				action->intret    = digits;
				action->type      = SHOW_DOUBLE_VALUE;
				action->lots      = value;
				action->openprice = 0;
				action->slprice   = 0;
				action->tpprice   = 0;
				action->actionId  = JOB_SHOW_VALUE;
				Mql::write2str(&action->comment, label.c_str());

				MARK_FUNC_OUT
			}

		private:
			
			int actionsLen;
			std::vector<TradeAction*> ext_tradeActions;

	};

}