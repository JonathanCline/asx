#pragma once

/**
 * @file
 * @brief Centralized format library include
*/

#include <jclib/feature.h>

#include <format>
#include <concepts>

namespace asx
{
	/**
	 * @brief Concept satisfied by types with a formatter specialization defined.
	*/
	template <typename T>
	concept cx_formattable = requires(const T & v)
	{
		std::formatter<T, char>{};
	};


	inline auto format(const std::string_view _fmt, const cx_formattable auto&... _args)
	{
		return std::vformat(_fmt, std::make_format_args(_args...));
	};

	/**
	 * @brief Formats or returns a string.
	 * 
	 * If a format string is provided along with arguments, this will return the formatted string.
	 * If only the format string is provided, it is returned as a `std::string` without modification.
	 * 
	 * @param _fmt Format string, only used for formatting if arguments are provided.
	 * @param ..._args Optional arguments used for formatting.
	 * 
	 * @return The formatted string if `_args` were given, otherwise returns the string unmodified.
	*/
	template <cx_formattable... Ts>
	inline std::string format_or_pass(const std::string& _fmtOrStr, const Ts&... _args)
	{
		if constexpr (sizeof...(Ts) == 0)
		{
			return _fmtOrStr;
		}
		else
		{
			return asx::format(_fmtOrStr, _args...);
		};
	};
};

namespace asx
{
	/**
	 * @brief Base class for help implementing the most common formatter types
	*/
	template <typename T>
	struct simple_formatter : private std::formatter<std::string_view, char>
	{
	private:
		using parse_context = std::basic_format_parse_context<char>;
	protected:
		template <typename CtxT>
		typename CtxT::iterator format_from_string(const std::string_view& _str, CtxT& _context) const
		{
			return std::formatter<std::string_view, char>::format(_str, _context);
		};
	public:
		constexpr typename parse_context::iterator parse(parse_context& _parseCtx)
		{
			auto it = _parseCtx.begin();
			// Ensure we hit the end
			if (it != _parseCtx.end() && *it != '}')
			{
				throw std::format_error("Missing '}' in format string.");
			};
			return it;
		};
		using std::formatter<std::string_view, char>::formatter;
	};
};
