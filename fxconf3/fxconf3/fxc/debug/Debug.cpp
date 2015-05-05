#pragma once

#include <sstream>
#include "Debug.h"

#if DEBUG

	__declspec(thread) std::vector< fxc::debug::SourcePoint >* fxc::debug::StackTrace::_callstack;

	void fxc::debug::StackTrace::init() {
		_callstack = new std::vector < fxc::debug::SourcePoint >;
	}
	void fxc::debug::StackTrace::flush() {
		delete _callstack;
	}
	void fxc::debug::StackTrace::push(const char *filename, int line, const std::string data) {
		_callstack->push_back(SourcePoint{ filename, line, data });
	}
	void fxc::debug::StackTrace::pop() {
		_callstack->pop_back();
	}
	std::string fxc::debug::StackTrace::trace() {
		std::ostringstream ss;
		ss << "Stack trace (" << _callstack->size() << "):\r\n";

		for (int i = 0, l = _callstack->size(); i < l; ++i) {
			if (_callstack->at(i).data == "") {
				ss << _callstack->at(i).filename << ":" << _callstack->at(i).line << "\r\n";
			}
			else {
				ss << _callstack->at(i).filename << ":" << _callstack->at(i).line << " {\r\n" << _callstack->at(i).data << "\r\n}\r\n";
			}
		}

		return ss.str();
	}

#endif