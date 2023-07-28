#pragma once

/** @file Defines a formatter for std::filesystem::path */

#include <format>
#include <filesystem>
#include <string_view>

namespace std
{
	/**
	 * @brief Formatter specialization for std filesystem path type.
	*/
	template <>
	struct formatter<std::filesystem::path, char> : std::formatter<std::string_view, char>
	{
	private:
		using parse_context = basic_format_parse_context<char>;
	public:
		constexpr typename parse_context::iterator parse(parse_context& _parseCtx)
		{
			auto it = _parseCtx.begin();
			
			//
			// Parse steps here, if any
			//

			// Ensure we hit the end
			if (it != _parseCtx.end() && *it != '}')
			{
				throw std::format_error("Missing '}' in format string.");
			};
			return it;
		};

		template <typename CtxT>
		typename CtxT::iterator format(const std::filesystem::path& _path, CtxT& _context) const
		{
			const auto _str = _path.string();
			return std::formatter<std::string_view, char>::format(_str, _context);
		};

		using std::formatter<std::string_view, char>::formatter;
	};
}