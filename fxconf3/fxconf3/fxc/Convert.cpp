#pragma once

#include "windows.h"
#include <string>
#include "Convert.h"

std::string fxc::utils::WC2MB(const wchar_t* buffer, int len) {
	int nChars = WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
	if (nChars == 0) return "";

	std::string newbuffer;
	newbuffer.resize(nChars);
	WideCharToMultiByte(CP_UTF8, 0, buffer, len, const_cast< char* >(newbuffer.c_str()), nChars, NULL, NULL);

	return newbuffer;
}

std::string fxc::utils::WC2MB(const std::wstring& str) {
	return fxc::utils::WC2MB(str.c_str(), (int) str.size());
}