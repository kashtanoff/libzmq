#include <sstream>
#include <iomanip>
#include <cstdarg>
#include "Format.h"

std::stringstream ss;

std::string fxc::utils::Format::decimal(double n, unsigned precision) {
	ss.str("");
	ss << std::setprecision(precision) << std::fixed << n;
	return ss.str();
}
std::string fxc::utils::Format::sformat(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	//char buf[255];

	int len = vsnprintf(nullptr, 0, fmt, ap);

	std::string result;
	result.resize(len);

	vsprintf(&result[0], fmt, ap);

	va_end(ap);

	return result;
}
