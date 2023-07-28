#pragma once

/** @file Provides some helper functions for serializing / deserializing time points */

#include <chrono>

namespace asx
{
	using utc_clock = std::chrono::utc_clock;
	using utc_time = std::chrono::utc_time<std::chrono::seconds>;

	utc_time get_utc_timestamp();



	using local_time = std::chrono::zoned_seconds;

	local_time utc_timestamp_to_local_time(utc_time _utcTime);
};