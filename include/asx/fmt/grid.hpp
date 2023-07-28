#pragma once

/** @file */

#include <asx/grid.hpp>
#include <asx/format.hpp>

#include <string>

namespace std
{
	template <asx::cx_formattable T>
	struct formatter<asx::basic_grid_pos<T>, char> : asx::simple_formatter<asx::basic_grid_pos<T>>
	{
		auto format(const asx::basic_grid_pos<T>& _value, auto& _ctx)
		{
			// TODO : Make faster with <charconv>
			auto _str = std::to_string(_value.x) + ", " + std::to_string(_value.y);
			return this->format_from_string(_str, _ctx);
		};
	};
}