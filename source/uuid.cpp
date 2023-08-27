#include <asx/uuid.hpp>

#include <asx/random.hpp>

#include <bit>
#include <array>
#include <cstddef>
#include <algorithm>

namespace asx
{
	uuid uuid::random()
	{
		auto _uuid = uuid{};
		auto it = _uuid.bytes_.begin();
		for (size_t n = 0; n != 4; ++n)
		{
			const auto qb = std::bit_cast<std::array<std::byte, 4>>(asx::random());
			it = std::copy(qb.begin(), qb.end(), it);
		};
		return _uuid;
	};

	constexpr std::byte hexchar_to_nibble(char c)
	{
		switch (c)
		{
		case '0': return static_cast<std::byte>(0x0);
		case '1': return static_cast<std::byte>(0x1);
		case '2': return static_cast<std::byte>(0x2);
		case '3': return static_cast<std::byte>(0x3);
		case '4': return static_cast<std::byte>(0x4);
		case '5': return static_cast<std::byte>(0x5);
		case '6': return static_cast<std::byte>(0x6);
		case '7': return static_cast<std::byte>(0x7);
		case '8': return static_cast<std::byte>(0x8);
		case '9': return static_cast<std::byte>(0x9);

		case 'a': return static_cast<std::byte>(0xA);
		case 'b': return static_cast<std::byte>(0xB);
		case 'c': return static_cast<std::byte>(0xC);
		case 'd': return static_cast<std::byte>(0xD);
		case 'e': return static_cast<std::byte>(0xE);
		case 'f': return static_cast<std::byte>(0xF);

		case 'A': return static_cast<std::byte>(0xA);
		case 'B': return static_cast<std::byte>(0xB);
		case 'C': return static_cast<std::byte>(0xC);
		case 'D': return static_cast<std::byte>(0xD);
		case 'E': return static_cast<std::byte>(0xE);
		case 'F': return static_cast<std::byte>(0xF);

		default:
			return static_cast<std::byte>(0xFF);
		};
	};

	constexpr std::array<char, 2> byte_to_hexchars(std::byte _bits)
	{
		constexpr std::array<char, 16> hexchars
		{
			'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'
		};

		const auto b = static_cast<uint8_t>(_bits);
		const auto nLo = b & 0x0F;
		const auto nHi = (b & 0xF0) >> 4;
		const auto cLo = hexchars.at(nLo);
		const auto cHi = hexchars.at(nHi);

		return { cHi, cLo };
	};

	inline auto parse_hexbyte(auto it, std::byte& b)
	{
		const auto cHi = *(it++);
		const auto cLo = *(it++);
		const auto nHi = hexchar_to_nibble(cHi);
		const auto nLo = hexchar_to_nibble(cLo);
		b = (nHi << 4) | nLo;
		return it;
	};

	constexpr bool is_hexchar(char c)
	{
		return (c >= '0' && c <= '9') ||
			(c >= 'a' && c <= 'f') ||
			(c >= 'A' && c <= 'F');
	};

	uuid uuid::parse(std::string_view _str)
	{
		if (_str.size() < 36)
		{
			return uuid::null();
		};

		if (!std::all_of(_str.begin(), _str.begin() + 8, is_hexchar))
		{
			return uuid::null();
		};
		if (_str[8] != '-')
		{
			return uuid::null();
		};
		if (!std::all_of(_str.begin() + 9, _str.begin() + 13, is_hexchar))
		{
			return uuid::null();
		};
		if (_str[13] != '-')
		{
			return uuid::null();
		};
		if (!std::all_of(_str.begin() + 14, _str.begin() + 18, is_hexchar))
		{
			return uuid::null();
		};
		if (_str[18] != '-')
		{
			return uuid::null();
		};
		if (!std::all_of(_str.begin() + 19, _str.begin() + 23, is_hexchar))
		{
			return uuid::null();
		};
		if (_str[23] != '-')
		{
			return uuid::null();
		};
		if (!std::all_of(_str.begin() + 24, _str.begin() + 36, is_hexchar))
		{
			return uuid::null();
		};

		auto o = uuid{};

		auto it = _str.begin();
		size_t bn = 0;
		for(size_t n = 0; n != 4; ++n)
		{
			it = parse_hexbyte(it, o.bytes_[bn++]);
		};

		++it;
		for(size_t n = 0; n != 3; ++n)
		{
			it = parse_hexbyte(it, o.bytes_[bn++]);
			it = parse_hexbyte(it, o.bytes_[bn++]);
			++it;
		};

		for(size_t n = 0; n != 6; ++n)
		{
			it = parse_hexbyte(it, o.bytes_[bn++]);
		};
		return o;
	};


	inline auto write_hexbyte(auto _outIt, std::byte _byte)
	{
		const auto _chars = byte_to_hexchars(_byte);
		*_outIt = _chars[0];
		++_outIt;
		*_outIt = _chars[1];
		++_outIt;
		return _outIt;
	};

	std::string uuid::str() const
	{
		auto _str = std::string(36, '\0');

		auto it = _str.begin();
		size_t bn = 0;
		for(size_t n = 0; n != 4; ++n)
		{
			it = write_hexbyte(it, this->get_byte(bn++));
		};

		*(it++) = '-';
		for(size_t n = 0; n != 3; ++n)
		{
			it = write_hexbyte(it, this->get_byte(bn++));
			it = write_hexbyte(it, this->get_byte(bn++));
			*(it++) = '-';
		};

		for (size_t n = 0; n != 6; ++n)
		{
			it = write_hexbyte(it, this->get_byte(bn++));
		};

		return _str;
	};

};