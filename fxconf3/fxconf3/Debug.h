#pragma once

#include "vector"
#include "Defines.h"

#if DEBUG

	namespace fxc {

	namespace debug {

		struct SourcePoint {
			const char *filename;
			int line;
			SourcePoint(const char *filename, int line);
		};

		class StackTrace {
			public:
				static void push(const char *filename, int line);
				static void pop();
				static std::string trace();
			private:
				static std::vector<SourcePoint> _callstack;
		};

	}

	}

	#define MARK_FUNC_IN fxc::debug::StackTrace::push(__FILE__, __LINE__);
	#define MARK_FUNC_OUT fxc::debug::StackTrace::pop();
	#define STACK_TRACE fxc::debug::StackTrace::trace()
#else
	#define MARK_FUNC_IN
	#define MARK_FUNC_OUT
	#define STACK_TRACE ""
#endif