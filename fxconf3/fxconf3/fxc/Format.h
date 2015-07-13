#pragma once

#include <string>

namespace fxc {

namespace utils {

	class Format {

		public:

			static std::string decimal(double n, unsigned precision);
			static std::string sformat(const char *fmt, ...);

	};

}

}