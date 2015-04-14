#pragma once

#include "vector"

#include "debug/Debug.h"
#include "fxc.h"
#include "../MqlUtils.cpp"

namespace fxc { 

	#pragma pack(1)
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

	class TradeManager {

		public:

			void mapActions(void* arrayPtr, int length) {
				auto ptr = (fxc::TradeAction*) arrayPtr;

				for (auto i = 0; i < length; ++i) {
					ext_tradeActions.push_back(ptr + i);
				}

				//fxc::msg << "array  mql::[0x" << pointer << "][" << length << "]\r\n" << fxc::msg_box;
				//fxc::msg << "vector [" << vec->size() << "] / [" << vec->capacity() << "]\r\n" << fxc::msg_box;
				//
				//for (auto i = 0; i < length; ++i) {
				//	vec->at(i)->o_lots      = i + 0; // 8 bytes
				//	vec->at(i)->o_openprice = i + 1; // 8 bytes
				//	vec->at(i)->o_tpprice   = i + 2; // 8 bytes
				//	vec->at(i)->o_slprice   = i + 3; // 8 bytes
				//	vec->at(i)->o_ticket    = i + 4; // 4 bytes
				//	vec->at(i)->o_type      = i + 5; // 4 bytes
				//	vec->at(i)->intret      = i + 6; // 4 bytes
				//
				//	std::stringstream ss;
				//	ss << "~> cmnt[" << i*i << "]";
				//
				//	Mql::write2str(&vec->at(i)->comment, ss.str().c_str());
				//
				//	fxc::msg 
				//		<< "vector[" << i << "] " 
				//		<< "[0x" << vec->at(i) << "] "
				//		//<< vec->at(i)->o_lots << ", "
				//		//<< vec->at(i)->o_openprice << ", "
				//		//<< vec->at(i)->o_tpprice << ", "
				//		//<< vec->at(i)->o_slprice << ", "
				//		//<< vec->at(i)->o_ticket << ", "
				//		//<< vec->at(i)->o_type << ", "
				//		//<< vec->at(i)->intret << ", "
				//		<< "["
				//		<< vec->at(i)->comment.size << ", "
				//		<< vec->at(i)->comment.buffer 
				//		<< "] : \"" << Mql::str2chars(&vec->at(i)->comment) << "\""
				//		<< "\r\n" << fxc::msg_box;
				//}
			}
			//Обнуляет список действий
			inline void resetActionManager() {
				actionsLen = 0;
			}

		protected:
			//Возвращает количество назначенных действий
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


#pragma endregion
		private:

			int actionsLen;

			std::vector<TradeAction*> ext_tradeActions;

	};

}