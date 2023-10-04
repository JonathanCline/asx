#pragma once

/** @file */

#include <asx/format.hpp>
#include <asx/os.hpp>

#ifdef _MSC_VER
	#ifndef NDEBUG
		#define ASX_DEBUG
	#endif
#else
	#error "Finish me, this should define ASX_DEBUG if debugging should be performed"
#endif

#ifdef ASX_OS_WINDOWS
	#define ASX_BREAK() __debugbreak()
#else
	namespace asx::impl
	{
		void debug_break_impl();
	};
	#define ASX_BREAK() ::asx::impl::debug_break_impl()
#endif

#ifdef ASX_DEBUG
	/**
	 * Breaks execution when hit if in debug mode.
	*/
	#define ASX_DEBUG_BREAK() ASX_BREAK()
#else
	/**
	 * Breaks execution when hit if in debug mode.
	*/
	#define ASX_DEBUG_BREAK() {}
#endif

namespace asx
{
	namespace impl
	{
		extern void (*abort_fn_)();
		extern void (*exit_fn_)(int);
	};

	[[noreturn]] inline void abort()
	{
		asx::impl::abort_fn_();
	};
	[[noreturn]] inline void exit(int _exitCode)
	{
		asx::impl::exit_fn_(_exitCode);
	};

	void notify_assertion_failure(const char* _cond);
	void notify_failure(const char* _cond);

	void notify_failure(const std::string& _message);
};


#define ASX_FAIL(reasonFormat, ...) { ::asx::notify_failure(::asx::format_or_pass(reasonFormat __VA_OPT__(,) __VA_ARGS__)); ASX_DEBUG_BREAK(); ::asx::exit(1); }

#ifdef ASX_DEBUG
	#define ASX_ASSERT(cond) { if(!(cond)) { \
		::asx::notify_assertion_failure("ASX_ASSERT condition failed " #cond); \
		ASX_DEBUG_BREAK(); ::asx::exit(1); }; }
#else
	#define ASX_ASSERT(cond) { }
#endif

// Same as ASX_ASSERT() but not disabled by debugging turned off
#define ASX_CHECK(cond) { if(!(cond)) {\
	::asx::notify_assertion_failure("ASX_CHECK condition failed " #cond); \
	ASX_DEBUG_BREAK(); ::asx::exit(1); }; }
