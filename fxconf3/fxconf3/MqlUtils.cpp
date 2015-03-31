#pragma once

#include "windows.h"

#pragma pack(push,1)
struct MqlString {
	int      size;     // 4 bytes
	LPWSTR   buffer;   // 4 bytes
	int      reserved; // 4 bytes
};
#pragma pack(pop,1)

#pragma pack(push,1)
struct MqlRates {
	time_t time;
	double open;
	double high;
	double low;
	double close;
	long   tick_volume;
	int    spread;
	long   real_volume;
};
#pragma pack(pop,1)

class Mql {
	
	public:

		static char* str2chars(MqlString* str) {
			auto psz = new char[wcslen(str->buffer) + 1];
			wsprintfA(psz, "%S", str->buffer);
			return psz;
		}

		inline static void write2str(MqlString* str, const char* chars) {
			write2str(str, chars, false);
		}

		inline static void write2str(MqlString* str, const char* chars, bool terminate) {
			mbstowcs(str->buffer, chars, str->size + terminate);
		}

};