#pragma once 

/** @file */

#include <span>
#include <array>
#include <string>
#include <cstddef>
#include <cstdint>
#include <compare>
#include <algorithm>
#include <string_view>

namespace asx
{
	struct uuid
	{
	private:

		using storage_type = std::array<std::byte, 16>;

	public:

		constexpr auto operator<=>(const uuid& rhs) const = default;

		/**
		 * @brief Gets the size of the UUID in bytes.
		 * @return Size in bytes.
		*/
		constexpr static size_t size_bytes()
		{
			return sizeof(storage_type);
		};

		/**
		 * @brief Returns an array of bytes representing the uuid value.
		 * @return Array of bytes with UUID encoded.
		*/
		constexpr storage_type to_bytes() const
		{
			return this->bytes_;
		};

		constexpr static bool from_bytes(std::span<const std::byte> _bytes, uuid& _outValue)
		{
			if (_bytes.size() < uuid::size_bytes()) { return false; };
			std::copy(_bytes.begin(), _bytes.begin() + uuid::size_bytes(), _outValue.bytes_.begin());
			return true;
		};
		
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
		
		storage_type bytes_;

	};
};