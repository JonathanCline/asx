#pragma once

/** @file */

#include <asx/format.hpp>

#include <span>
#include <string>

namespace std
{
	template <asx::cx_formattable T, size_t Extent>
	struct formatter<std::span<T, Extent>, char> : asx::simple_formatter<std::span<T, Extent>>
	{
		auto format(const std::span<T, Extent>& _value, auto& _ctx)
		{
			// TODO : Make faster
			std::string _str = "{";

			bool _isFirst = true;
			for (const auto& v : _value)
			{
				std::string_view _fmt = ",{}";
				if (_isFirst)
				{
					_fmt = "{}";
					_isFirst = false;
				};
				_str.append(asx::format(_fmt, v));
			};
			_str += '}';
			return this->format_from_string(_str, _ctx);
		};
	};
}