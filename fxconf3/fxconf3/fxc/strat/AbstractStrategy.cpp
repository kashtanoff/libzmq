#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace strategy {

	class AbstractStrategy : public fxc::Simbiot {

		public:

			AbstractStrategy(char* _symbol) : Simbiot(_symbol) {}

			// возвращает количество операций на текущем тике
			virtual int getJob() = 0;

		protected:



	};

}

}