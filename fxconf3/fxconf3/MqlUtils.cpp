#pragma once

#include "windows.h"
#include <string>

#pragma pack(push,1)
struct MqlString {
	int      size;     // 4 bytes
	LPWSTR   buffer;   // 4 bytes
	int      reserved; // 4 bytes
};
#pragma pack(pop,1)

#pragma pack(push,1)
struct MqlRates {
	__time64_t time;
	double     open;
	double     high;
	double     low;
	double     close;
	__int64    tick_volume;
	int        spread;
	__int64    real_volume;
};
#pragma pack(pop,1)


class Mql {
	
	public:

		static char* str2chars(MqlString* str) {
			auto psz = new char[wcslen(str->buffer) + 1];
			wsprintfA(psz, "%S", str->buffer);
			return psz;
		}

		inline static void write2str(MqlString* mqlstr, const char* chars) {
			mbstowcs(mqlstr->buffer, chars, mqlstr->size);
		}

		inline static void write2str(MqlString* mqlstr, const std::string& str) {
			//write2str(mqlstr, str.c_str());
			mbstowcs(mqlstr->buffer, str.c_str(), str.length()+1);
		}

};