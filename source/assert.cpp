#include <asx/assert.hpp>
#include <asx/os.hpp>
#include <asx/logging.hpp>

#include <string>
#include <cstdlib>
#include <iostream>

#ifdef ASX_OS_WINDOWS
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#else
	#include <csignal>

	namespace asx::impl
	{
		void debug_break_impl()
		{
			::raise(SIGTRAP);
		};
	};
#endif

namespace asx
{
	namespace impl
	{
		void (*abort_fn_)() = &::abort;
		void (*exit_fn_)(int) = &::exit;
	};

	void notify_assertion_failure(const char* _cond)
	{
		asx::log_fatal_error(asx::get_stack_trace(1), _cond);

#ifdef ASX_OS_WINDOWS
		MessageBoxA(nullptr, _cond, "Fatal Error", MB_OK);
#endif
	};

	void notify_failure(const char* _reason)
	{
		asx::log_fatal_error(asx::get_stack_trace(1), _reason);

#ifdef ASX_OS_WINDOWS
		MessageBoxA(nullptr, _reason, "Fatal Error", MB_OK);
#endif
	};
	void notify_failure(const std::string& _reason)
	{
		asx::log_fatal_error(asx::get_stack_trace(1), _reason);

#ifdef ASX_OS_WINDOWS
		MessageBoxA(nullptr, _reason.c_str(), "Fatal Error", MB_OK);
#endif
	};
};
