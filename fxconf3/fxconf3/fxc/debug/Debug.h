#pragma once

#include "vector"
#include "../../Defines.h"

#if DEBUG

	namespace fxc {

	namespace debug {

		struct SourcePoint {
			const char *filename;
			int line;
			const std::string data;

			SourcePoint(const char *filename, int line, const std::string data);
		};

		class StackTrace {
			public:
				static void push(const char *filename, int line, std::string data = "");
				static void pop();
				static std::string trace();
			private:
				static std::vector<SourcePoint> _callstack;
		};

	}

	}

	#define MARK_FUNC_IN      fxc::debug::StackTrace::push(__FILE__, __LINE__);
	#define MARK_FUNC_IN_T(t) fxc::debug::StackTrace::push(__FILE__, __LINE__, t);
	#define MARK_FUNC_OUT     fxc::debug::StackTrace::pop();
	#define STACK_TRACE       fxc::debug::StackTrace::trace()
#else
	#define MARK_FUNC_IN
	#define MARK_FUNC_IN_T(t) 
	#define MARK_FUNC_OUT
	#define STACK_TRACE ""
#endif