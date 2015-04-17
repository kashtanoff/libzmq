#include <sstream>
#include <iomanip>
#include "Format.h"

std::stringstream ss;

std::string fxc::utils::Format::decimal(double n, unsigned precision) {
	ss.str("");
	ss << std::setprecision(precision) << std::fixed << n;
	return ss.str();
}
