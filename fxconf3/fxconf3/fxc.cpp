#pragma once

#include <io.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <windows.h>
#include <fcntl.h>
#include <mutex>

namespace fxc {

	static std::mutex mutex;

	bool	first_run = true;
	int		sign[6]   = { 1, -1, 1, -1, 1, -1 }; //Знаки для разных типов операций
	double	minmax[2] = { 0.0, 1000000.0 };      //дает начальное число для сортировки цен на покупку и продажу
	double	maxmin[2] = { 1000000.0, 0.0 };      //тоже, но наоборот

	bool   bp;
	bool   console;
	int    curH;
	time_t rawtime;

	static const WORD MAX_CONSOLE_LINES = 500;

	void RedirectIOToConsole()
	{
		int  hConHandle;
		long lStdHandle;
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
		FILE* fp;
		AllocConsole(); // allocate a console for this app

		// set the screen buffer to be big enough to let us scroll text
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
		coninfo.dwSize.Y = MAX_CONSOLE_LINES;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

		// redirect unbuffered STDOUT to the console
		lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stdout = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);

		// redirect unbuffered STDIN to the console
		lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "r");
		*stdin = *fp;
		setvbuf(stdin, NULL, _IONBF, 0);

		// redirect unbuffered STDERR to the console
		lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		fp = _fdopen(hConHandle, "w");
		*stderr = *fp;
		setvbuf(stderr, NULL, _IONBF, 0);

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
		// point to console as well
		std::ios_base::sync_with_stdio();
	}

	using std::ostringstream;
	using std::ostream;

	ostream& msg_box(ostream& s)
	{
		ostringstream& os = dynamic_cast<ostringstream&>(s);
		if (bp)
		{
			MessageBoxA(NULL, os.str().c_str(), "fxconf3.dll ", MB_OK);
			bp = false;
		}
		if (console)
		{
			time(&rawtime);
			std::cout << asctime(localtime(&rawtime)) << " - (" << curH << ")\r\n" << os.str().c_str() << "\r\n";
		}
		os.swap(ostringstream());
		return s;
	}
	ostream& clear(ostream& s)
	{
		ostringstream& os = dynamic_cast<ostringstream&>(s);
		os.swap(ostringstream());
		return s;
	}
	ostringstream msg;

}