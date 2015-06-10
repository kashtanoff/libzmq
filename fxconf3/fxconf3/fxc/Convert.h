#pragma once

namespace fxc {

namespace utils {

	std::string WC2MB(const wchar_t* buffer, int len);
	std::string WC2MB(const std::wstring& str);

}

}