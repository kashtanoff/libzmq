#pragma once

#include <io.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <windows.h>
#include <mutex>

namespace fxc {

	extern std::mutex mutex;

	extern bool   first_run;
	extern int    sign[6];   //Знаки для разных типов операций
	extern double minmax[2]; //дает начальное число для сортировки цен на покупку и продажу
	extern double maxmin[2]; //тоже, но наоборот

	extern bool   console;
	extern time_t rawtime;

	const WORD MAX_CONSOLE_LINES = 10000;

	void RedirectIOToConsole();

	std::ostream& msg_box(std::ostream& s);
	std::ostream& clear(std::ostream& s);
	extern std::ostringstream msg;

}