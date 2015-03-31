#pragma once

#include "../Simbiot.cpp"

namespace fxc {

namespace indicator {

	class AbstractIndicator {

		public:

			virtual void compute() = 0;

	};

}

}