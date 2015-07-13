#pragma once

#include "vector"
#include "../../Defines.h"

#if DEBUG

	namespace fxc {

	namespace debug {

		struct SourcePoint {
			const char *filename;
			const int line;
			const std::string data;
		};

		class StackTrace {
			public:
				static void init();
				static void flush();
				static void clear();
				static void push(const char *filename, int line, std::string data = "");
				static void last(const char *filename, int line, std::string data = "");
				static void pop();
				static std::string trace();
			private:
				static __declspec(thread) std::vector< SourcePoint >* _callstack;
				static __declspec(thread) SourcePoint* _sourcepoint;
		};

	}

	}

	#define MARK_FUNC_IN      fxc::debug::StackTrace::push(__FILE__, __LINE__);
	#define MARK_FUNC_IN_T(t) fxc::debug::StackTrace::push(__FILE__, __LINE__, t);
	#define MARK_POINT		  fxc::debug::StackTrace::last(__FILE__, __LINE__);
	#define MARK_FUNC_OUT     fxc::debug::StackTrace::pop();
	#define STACK_TRACE_INIT  fxc::debug::StackTrace::init();
	#define STACK_TRACE_FLUSH fxc::debug::StackTrace::flush();
	#define STACK_TRACE_CLEAR fxc::debug::StackTrace::clear();
	#define STACK_TRACE       fxc::debug::StackTrace::trace()
#else
	#define MARK_FUNC_IN
	#define MARK_FUNC_IN_T(t) 
	#define MARK_POINT
	#define MARK_FUNC_OUT
	#define STACK_TRACE_INIT
	#define STACK_TRACE_CLEAR
	#define STACK_TRACE_FLUSH
	#define STACK_TRACE ""
#endif