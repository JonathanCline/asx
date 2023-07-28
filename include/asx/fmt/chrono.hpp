#pragma once

/** @file Provides some helper functions for serializing / deserializing time points */

#include <asx/time.hpp>

#include <string>
#include <optional>
#include <string_view>

namespace asx
{
	std::string utc_timestamp_to_string(utc_time _time);
	std::optional<utc_time> string_to_utc_timestamp(std::string_view _str);
};