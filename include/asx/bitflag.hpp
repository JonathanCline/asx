#pragma once

/**
 * @file
 * @brief Provides a tool for easily defining bitflag types using an enum to help name the fields.
*/

#include <jclib/concepts.h>
#include <jclib/type_traits.h>

namespace asx
{
	template <typename EnumT> requires std::is_enum_v<EnumT>
	class basic_bitflag
	{
	public:

		/**
		 * @brief The underlying enumerator type used to define the individual flag bits.
		*/
		using enum_type = EnumT;

		constexpr operator enum_type() const noexcept
		{
			return this->value_;
		};

		constexpr friend static basic_bitflag operator~(const basic_bitflag& rhs) noexcept
		{
			return basic_bitflag(static_cast<enum_type>(~jc::to_underlying(rhs.value_)));
		};

		constexpr friend static bool operator==(const basic_bitflag& lhs, const basic_bitflag& rhs) noexcept = default;

		constexpr friend static basic_bitflag operator|(const basic_bitflag& lhs, const basic_bitflag& rhs) noexcept
		{
			return basic_bitflag(static_cast<enum_type>(jc::to_underlying(lhs.value_) | jc::to_underlying(rhs.value_)));
		};
		constexpr friend static basic_bitflag operator&(const basic_bitflag& lhs, const basic_bitflag& rhs) noexcept
		{
			return basic_bitflag(static_cast<enum_type>(jc::to_underlying(lhs.value_) & jc::to_underlying(rhs.value_)));
		};
		constexpr friend static basic_bitflag operator^(const basic_bitflag& lhs, const basic_bitflag& rhs) noexcept
		{
			return basic_bitflag(static_cast<enum_type>(jc::to_underlying(lhs.value_) ^ jc::to_underlying(rhs.value_)));
		};
		constexpr friend static basic_bitflag& operator|=(basic_bitflag& lhs, const basic_bitflag& rhs) noexcept
		{
			lhs = lhs | rhs;
			return lhs;
		};
		constexpr friend static basic_bitflag& operator&=(basic_bitflag& lhs, const basic_bitflag& rhs) noexcept
		{
			lhs = lhs & rhs;
			return lhs;
		};
		constexpr friend static basic_bitflag& operator^=(basic_bitflag& lhs, const basic_bitflag& rhs) noexcept
		{
			lhs = lhs ^ rhs;
			return lhs;
		};
		
		template <std::integral T>
		constexpr friend static basic_bitflag operator<<(const basic_bitflag& lhs, T n)
		{
			return basic_bitflag(static_cast<enum_type>(jc::to_underlying(lhs.value_) << n));
		};
		template <std::integral T>
		constexpr friend static basic_bitflag operator>>(const basic_bitflag& lhs, T n)
		{
			return basic_bitflag(static_cast<enum_type>(jc::to_underlying(lhs.value_) >> n));
		};
		template <std::integral T>
		constexpr friend static basic_bitflag& operator<<=(basic_bitflag& lhs, T n)
		{
			lhs = lhs << n;
			return lhs;
		};
		template <std::integral T>
		constexpr friend static basic_bitflag& operator>>=(basic_bitflag& lhs, T n)
		{
			lhs = lhs >> n;
			return lhs;
		};

		/**
		 * @brief Gets the raw enum value held by this bitflag.
		 * @return Held enum value.
		*/
		constexpr enum_type value() const noexcept
		{
			return this->value_;
		};

		/**
		 * @brief Tests if all of the given flags are set.
		 * @param _flags Flag bits to test.
		 * @return True if all given flags are set, false if any of them are not set.
		*/
		constexpr bool all(basic_bitflag _flags) const noexcept
		{
			return (*this & _flags) == _flags;
		};

		/**
		 * @brief Tests if one or more of the given flags are set.
		 * @param _flags Flag bits to test.
		 * @return True if one or more of the given flags are set, false if none of them are set.
		*/
		constexpr bool any(basic_bitflag _flags) const noexcept
		{
			return (*this & _flags) != basic_bitflag{};
		};

		/**
		 * @brief Tests if none of the given flags are set.
		 * @param _flags Flag bits to test.
		 * @return True if none of the given flags are set, false if any of them are set.
		*/
		constexpr bool none(basic_bitflag _flags) const noexcept
		{
			return (*this & _flags) == basic_bitflag{};
		};

		/**
		 * @brief Tests if ONLY the given flags are set.
		 * @param _flags Flag bits to test.
		 * @return True if all of the given flags are the only ones set, false if any of them are NOT set or if a flag
		 * not in `_flags` was set.
		*/
		constexpr bool only(basic_bitflag _flags) const noexcept
		{
			return *this == _flags;
		};

		/**
		 * @brief Sets the given flags.
		 * @param _flags Flag bits to set.
		*/
		constexpr void set(basic_bitflag _flags) noexcept
		{
			this->value_ = static_cast<enum_type>(jc::to_underlying(this->value_) | jc::to_underlying(_flags.value_));
		};

		/**
		 * @brief Clears the given flags.
		 * @param _flags Flag bits to clear.
		*/
		constexpr void clear(basic_bitflag _flags) noexcept
		{
			this->value_ = static_cast<enum_type>(jc::to_underlying(this->value_) & ~jc::to_underlying(_flags.value_));
		};

		constexpr basic_bitflag() = default;
		constexpr basic_bitflag(enum_type _value) noexcept :
			value_(_value)
		{};
		constexpr basic_bitflag& operator=(enum_type _value) noexcept
		{
			this->value_ = _value;
			return *this;
		};

		enum_type value_;
	};
};
