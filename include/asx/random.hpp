#pragma once

/** @file */

#include <jclib/concepts.h>

#include <cstdint>

namespace asx
{
	struct random_t
	{
	private:
		template <typename T>
		static T invoke_impl();
	public:
		
		template <typename T>
		requires (jc::cx_integer<T> || jc::cx_floating_point<T>)
		static T invoke()
		{
			return invoke_impl<T>();
		};

		uint32_t operator()() const { return this->invoke<uint32_t>(); };
	};
	constexpr inline random_t random{};
};
