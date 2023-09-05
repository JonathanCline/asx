#pragma once

/** @file */

#include <jclib/concepts.h>

#include <list>
#include <mutex>
#include <optional>

namespace asx
{
	/**
	 * @brief Provides a thread-safe SINGLE READER SINGLE WRITER message queue (FIFO).
	 * 
	 * WARNING: This is designed for one thread to write and one thread to read. While
	 * the initial implementation isn't directly optimized for this, all thread safety
	 * guarentees rely on this being used as a SINGLE READER SINGLE WRITER queue.
	 * 
	 * @tparam T The type the message queue will store.
	*/
	template <jc::cx_move_constructible T>
	class message_queue
	{
	public:

		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;

		using size_type = size_t;

	private:

		/**
		 * @brief Acquires a unique_lock on the mutex.
		 * @return Unique lock owning the locked mutex.
		*/
		[[nodiscard]] auto acquire_lock() const
		{
			return std::unique_lock(this->mtx_);
		};

	public:
		
		/**
		 * @brief Checks if the queue has NO data queued up.
		 * @return True if the next call to try_next() would return null.
		*/
		bool empty() const
		{
			const auto lck = this->acquire_lock();
			return this->data_.empty();
		};

		/**
		 * @brief Clears all data from the queue.
		*/
		void clear() noexcept
		{
			const auto lck = this->acquire_lock();
			this->data_.clear();
		};
		
		/**
		 * @brief Attempts to grab the next element in the queue.
		 * 
		 * This will pop the element from the queue if there is one to grab.
		 * 
		 * @return The next element in the queue, or nullopt if the queue is empty.
		*/
		std::optional<value_type> try_next()
		{
			const auto lck = this->acquire_lock();
			if (!this->data_.empty())
			{
				auto _data = std::move(this->data_.front());
				this->data_.pop_front();
				return _data;
			}
			else
			{
				return std::nullopt;
			};
		};

		/**
		 * @brief Pushes an element onto the queue by copy.
		 * @param _value Value to add to the queue.
		*/
		void push(const_reference _value)
		{
			const auto lck = this->acquire_lock();
			this->data_.push_back(_value);
		};

		/**
		 * @brief Pushes an element onto the queue by move.
		 * @param _value Value to add to the queue.
		*/
		void push(value_type&& _value)
		{
			const auto lck = this->acquire_lock();
			this->data_.push_back(std::move(_value));
		};

		/**
		 * @brief Constructs an empty message queue.
		*/
		message_queue() = default;

	private:

		/**
		 * @brief The mutex used to protect the actual queue data structure.
		*/
		mutable std::mutex mtx_;

		/**
		 * @brief The underlying queue data structure.
		*/
		std::list<value_type> data_;
	};
};