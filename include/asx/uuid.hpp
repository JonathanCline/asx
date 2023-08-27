#pragma once 

/** @file */

#include <array>
#include <string>
#include <cstddef>
#include <cstdint>
#include <compare>
#include <string_view>

namespace asx
{
	struct uuid
	{
	public:

		constexpr auto operator<=>(const uuid& rhs) const = default;

		
		constexpr bool is_null() const
		{
			for (auto& v : this->bytes_)
			{
				if (v != std::byte{}) { return false; };
			};
			return true;
		};
		constexpr explicit operator bool() const
		{
			return !this->is_null();
		};

		constexpr static uuid null() noexcept
		{
			return uuid{};
		};
		static uuid random();

		// Returns null UUID on failure.
		static uuid parse(std::string_view _uuid);

		std::string str() const;

		/**
		 * @brief Gets the size of the UUID in bytes.
		 * @return Size in bytes.
		*/
		constexpr size_t size_bytes() const
		{
			return this->bytes_.size();
		};

		constexpr uuid() noexcept = default;
	private:

		constexpr std::byte get_byte(size_t n) const
		{
			return this->bytes_.at(n);
		};
		
		constexpr void set_byte(std::byte _byte, size_t n)
		{
			this->bytes_.at(n) = _byte;
		};
		
		std::array<std::byte, 16> bytes_;

	};
};