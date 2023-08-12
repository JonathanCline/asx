#pragma once

/** @file */

#include <span>
#include <array>

namespace asx
{
	template <typename T, size_t N>
	class ring_buffer
	{
	public:
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;

	private:
		using container_type = std::array<value_type, N>;
	public:
		auto to_span() const { return std::span(this->data_); };

		auto raw_begin() { return this->data_.begin(); };
		auto raw_begin() const { return this->data_.cbegin(); };
		auto raw_end() { return this->data_.end(); };
		auto raw_end() const { return this->data_.cend(); };
		
		auto size() const { return this->data_.size(); };
		void push(const_reference _value)
		{
			this->data_.at(this->head_++) = _value;
			if (this->head_ >= N)
			{
				this->head_ = 0;
			};
		};

		constexpr ring_buffer() = default;
		constexpr explicit ring_buffer(const_reference _fillValue)
		{
			this->data_.fill(_fillValue);
		};

	private:
		std::array<value_type, N> data_{};
		size_t head_ = 0;
	};

};