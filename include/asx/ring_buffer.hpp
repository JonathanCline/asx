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

		using size_type = size_t;

		auto begin() { return this->data_.begin(); };
		auto begin() const { return this->data_.cbegin(); };
		auto end() { return this->data_.end(); };
		auto end() const { return this->data_.cend(); };
		
		size_type capacity() const
		{
			return this->data_.size();
		};
		size_type size() const
		{
			return this->data_.size();
		};
		
		void push(const_reference _value)
		{
			const auto _elementPos = this->head_pos_;
			this->data_.at(_elementPos) = _value;
			
			++this->head_pos_;
			if (this->head_pos_ >= N)
			{
				this->head_pos_ = 0;
			};
		};

		constexpr ring_buffer() = default;
		constexpr explicit ring_buffer(const_reference _fillValue)
		{
			this->data_.fill(_fillValue);
			this->head_pos_ = 0;
		};

	private:
		std::array<value_type, N> data_{};

		/**
		 * @brief The position that the next written element will be at.
		*/
		size_t head_pos_ = 0;
	};

};