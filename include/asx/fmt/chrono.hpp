#pragma once

/** @file Provides some helper functions for serializing / deserializing time points */

#include <asx/time.hpp>
#include <asx/format.hpp>

#include <string>
#include <optional>
#include <string_view>

namespace asx
{
	std::string utc_timestamp_to_string(utc_time _time);
};

#ifdef ASX_OS_LINUX
namespace std
{
	template <>
	struct formatter<::asx::utc_time, char> : public ::asx::simple_formatter<::asx::utc_time>
	{
		template <typename CtxT>
		typename CtxT::iterator format(const ::asx::utc_time& _value, CtxT& _context) const
		{
			auto _str = ::asx::utc_timestamp_to_string(_value);
			return this->format_from_string(_str, _context);
		};

		using ::asx::simple_formatter<::asx::utc_time>::simple_formatter;
	};
}
#endif