#pragma once

#include <io.h>
#include <iostream>
#include <sstream>
#include <thread>

#include <windows.h>
#include <fcntl.h>
#include "fxc.h"

std::mutex fxc::mutex;

bool   fxc::console;
time_t fxc::rawtime;

void fxc::RedirectIOToConsole()
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

ostream& fxc::msg_box(ostream& s)
{
	ostringstream& os = dynamic_cast<ostringstream&>(s);

	if (console) {
		char buffer[32];

		time(&rawtime);
		strftime(buffer, 32, "%H:%M:%S", localtime(&rawtime));

		std::cout << buffer << " [" << std::this_thread::get_id() << "]: " << os.str().c_str();
	}
	
	os.swap(ostringstream());
	return s;
}
ostream& fxc::clear(ostream& s)
{
	ostringstream& os = dynamic_cast<ostringstream&>(s);
	os.swap(ostringstream());
	return s;
}
std::ostringstream fxc::msg;