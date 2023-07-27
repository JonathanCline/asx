#pragma once

/** @file */

#include <concepts>
#include <type_traits>

namespace asx
{
	template <typename T, typename MatchToT>
	struct match_const { using type = T; };

	template <typename T, typename MatchToT>
	struct match_const<const T, MatchToT> { using type = T; };

	template <typename T, typename MatchToT>
	struct match_const<T, const MatchToT> { using type = const T; };

	template <typename T, typename MatchToT>
	struct match_const<const T, const MatchToT> { using type = const T; };


	template <typename T, typename MatchToT>
	using match_const_t = typename match_const<T, MatchToT>::type;


	struct immobile
	{
	public:
		immobile() = default;
		~immobile() = default;
	private:
		immobile(const immobile&) = delete;
		immobile& operator=(const immobile&) = delete;
		immobile(immobile&&) noexcept = delete;
		immobile& operator=(immobile&&) noexcept = delete;
	};


	template <auto V>
	struct cxconstant { using type = decltype(V); constexpr static type value = V; };

#define ASX_DEFINE_ITERATOR_FUNCTIONS(container) \
auto begin() { return container.begin(); }; \
auto begin() const { return container.begin(); }; \
auto cbegin() const { return container.begin(); }; \
auto end() { return container.end(); }; \
auto end() const { return container.end(); }; \
auto cend() const { return container.end(); }



}

namespace asx
{
	/**
	 * @brief Checks if the [BasicLockable](https://en.cppreference.com/w/cpp/named_req/BasicLockable) requirement is fufilled.
	 *
	 * @tparam T Type to check.
	*/
	template <typename T>
	concept cx_basic_lockable = requires(T a)
	{
		a.lock();
		a.unlock();
	};

	/**
	 * @brief Checks if the [BasicLockable](https://en.cppreference.com/w/cpp/named_req/BasicLockable) requirement is fufilled.
	 *
	 * @tparam T Type to check.
	*/
	template <typename T, typename = void>
	struct is_basic_lockable : std::bool_constant<cx_basic_lockable<T>> {};

	/**
	 * @copydoc asx::is_basic_lockable
	 *
	 * @tparam T Type to check.
	*/
	template <typename T>
	constexpr inline auto is_basic_lockable_v = is_basic_lockable<T>::value;

};


namespace asx
{
	/**
	 * @brief Checks if the [Lockable](https://en.cppreference.com/w/cpp/named_req/Lockable) requirement is fufilled.
	 *
	 * @tparam T Type to check.
	*/
	template <typename T>
	concept cx_lockable = cx_basic_lockable<T> && requires(T a)
	{
		{ a.try_lock() } -> std::same_as<bool>;
	};

	/**
	 * @brief Checks if the [Lockable](https://en.cppreference.com/w/cpp/named_req/Lockable) requirement is fufilled.
	 *
	 * @tparam T Type to check.
	*/
	template <typename T, typename = void>
	struct is_lockable : std::bool_constant<cx_lockable<T>> {};

	/**
	 * @copydoc asx::is_lockable
	 *
	 * @tparam T Type to check.
	*/
	template <typename T>
	constexpr inline auto is_lockable_v = is_lockable<T>::value;
};
