#pragma once

/** @file Provides some helper functions for serializing / deserializing time points */

#include <asx/os.hpp>

#include <ctime>
#include <chrono>

namespace asx
{
	struct utc_time_traits
	{
#ifdef ASX_OS_WINDOWS
		using rep = std::chrono::utc_clock::rep;
		using duration = std::chrono::utc_clock::duration;
		using time_point = std::chrono::utc_time<std::chrono::milliseconds>;
#else
		using rep = int64_t;
		using duration = std::chrono::duration<rep, std::milli>;
		using time_point = std::chrono::time_point<utc_time_traits, duration>;
#endif
	};
	using utc_time = utc_time_traits::time_point;

	utc_time get_utc_timestamp();

	struct local_time_traits
	{
#ifdef ASX_OS_WINDOWS
		using local_time = std::chrono::zoned_seconds;
#else
		using local_time = std::chrono::time_point<local_time_traits, std::chrono::seconds>;
#endif
	};
	using local_time = local_time_traits::local_time;

	local_time utc_timestamp_to_local_time(utc_time _utcTime);
};