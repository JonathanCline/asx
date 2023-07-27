#pragma once

/** @file */

#include <asx/assert.hpp>
#include <asx/exception.hpp>
#include <asx/type_traits.hpp>

#include <mutex>

namespace asx
{
	class not_locked_exception : public exception { public: using exception::exception; };


	/**
	 * @brief Holds a pointer and a unique lock with ownership of a mutex for the value pointed to.
	 * @tparam T Type to point to.
	 * @tparam MutexT Lockable type.
	*/
	template <typename T, cx_basic_lockable MutexT = std::mutex>
	struct locked_ptr
	{
	public:
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;

		using mutex_type = MutexT;

	private:
		using lock_type = std::unique_lock<mutex_type>;

	public:

		bool good() const
		{
			return static_cast<bool>(this->lck_);
		};
		explicit operator bool() const
		{
			return this->good();
		};

		/**
		 * @brief Unlocks the managed lock (if present) and releases ownership of the locked state.
		*/
		void unlock()
		{
			if (this->lck_)
			{
				this->ptr_ = nullptr;
				this->lck_.unlock();
			};
		};

		pointer get() const noexcept
		{
			return this->ptr_;
		};
		pointer operator->() const noexcept
		{
			return this->get();
		};
		reference operator*() const
		{
			if (!this->good())
			{
				ASX_THROW_EXCEPTION(not_locked_exception, "lock is not owned");
			};
			return *this->get();
		};

		explicit locked_ptr(reference _ptr, mutex_type& _mtx) :
			ptr_(&_ptr), lck_(_mtx)
		{
			ASX_CHECK((this->ptr_ != nullptr) == (this->lck_.owns_lock()));
		};
		
		locked_ptr(locked_ptr&& other) noexcept
		requires std::move_constructible<lock_type> :
			lck_(std::move(other.mtx_)),
			ptr_(std::exchange(other.ptr_, nullptr))
		{};

		locked_ptr& operator=(locked_ptr&& other) noexcept
		requires std::is_move_assignable_v<lock_type>
		{
			if (this == &other) { return *this; };

			this->unlock();
			this->lck_ = std::move(other.mtx_);
			this->ptr_ = std::exchange(other.ptr_, nullptr);
			return *this;
		};

		~locked_ptr()
		{
			this->unlock();
		};

	private:
		pointer ptr_;
		std::unique_lock<mutex_type> lck_;
	
		locked_ptr(const locked_ptr&) = delete;
		locked_ptr& operator=(const locked_ptr&) = delete;

	};

	/**
	 * @brief Protects an object with a lock - typically a mutex.
	 * @tparam T Type to allow exclusive access to.
	 * @tparam MutexT Lockable type used to enforce exclusive access.
	*/
	template <typename T, cx_basic_lockable MutexT = std::mutex>
	struct exclusive
	{
	public:
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const value_type*;
		using const_reference = const value_type&;

		using mutex_type = MutexT;

		using locked_type = asx::locked_ptr<value_type, mutex_type>;
		using const_locked_type = asx::locked_ptr<const value_type, mutex_type>;

	private:

		/**
		 * @brief Locks this object using the mutex type.
		*/
		void lock()
		{
			this->mtx_.lock();
		};

		/**
		 * @brief Unlocks this object using the mutex type.
		*/
		void unlock() noexcept
		{
			this->mtx_.unlock();
		};

	public:

		/**
		 * @brief Locks and then returns a handle that allows access to the managed object.
		 *
		 * May block if this is already locked.
		 *
		 * @return Locked pointer to the managed object.
		*/
		locked_type get()
		{
			return locked_type(this->value_, this->mtx_);
		};

		/**
		 * @brief Locks and then returns a handle that allows access to the managed object.
		 *
		 * May block if this is already locked.
		 *
		 * @return Locked pointer to the managed object.
		*/
		const_locked_type get() const
		{
			return const_locked_type(this->value_, this->mtx_);
		};

		/**
		 * @brief Locks and then returns a handle that allows access to the managed object.
		 * 
		 * May block if this is already locked.
		 * 
		 * @return Locked pointer to the managed object.
		*/
		locked_type operator->()
		{
			return this->get();
		};

		/**
		 * @brief Locks and then returns a handle that allows access to the managed object.
		 *
		 * May block if this is already locked.
		 *
		 * @return Locked pointer to the managed object.
		*/
		const_locked_type operator->() const
		{
			return this->get();
		};

		/**
		 * @brief Default initializes the managed object.
		*/
		exclusive() requires std::default_initializable<value_type> :
			value_{},
			mtx_{}
		{};

		/**
		 * @brief Initializes the managed object by copy.
		 * @param _value The value to copy.
		*/
		explicit exclusive(const_reference _value)
			requires std::copy_constructible<value_type> :
			value_(_value),
			mtx_{}
		{};

		/**
		 * @brief Initializes the managed object by move.
		 * @param _value The value to move.
		*/
		explicit exclusive(value_type&& _value)
			requires std::move_constructible<value_type> :
			value_(std::move(_value)),
			mtx_{}
		{};

	private:
		mutable mutex_type mtx_;
		value_type value_;
	};
};