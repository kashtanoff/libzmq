#pragma once

#include <sstream>
#include "Debug.h"

#if DEBUG

	fxc::debug::SourcePoint::SourcePoint(const char *filename, int line) : filename(filename), line(line) {}

	std::vector<fxc::debug::SourcePoint> fxc::debug::StackTrace::_callstack;
	void fxc::debug::StackTrace::push(const char *filename, int line) {
		_callstack.push_back(SourcePoint(filename, line));
	}
	void fxc::debug::StackTrace::pop() {
		_callstack.pop_back();
	}
	std::string fxc::debug::StackTrace::trace() {
		std::ostringstream ss;
		ss << "Stack trace (" << _callstack.size() << "):\r\n";

		for (int i = 0, l = _callstack.size(); i < l; ++i) {
			ss << _callstack[i].filename << ":" << _callstack[i].line << "\r\n";
		}

		return ss.str();
	}

#endif