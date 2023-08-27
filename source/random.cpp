#include <asx/random.hpp>

#include <limits>
#include <cstdint>
#include <random>
#include <numeric>
#include <algorithm>
#include <cmath>

namespace asx
{
	namespace impl
	{
		struct random_generator
		{
		private:
			using random_engine = std::mt19937;
		public:
			
			uint32_t random_uint()
			{
				static_assert(std::is_same_v<random_engine::result_type, uint32_t>);
				return this->mt();
			};

			template <typename T>
			T random()
			{
				using lim = std::numeric_limits<T>;
				using slim = std::numeric_limits<uint32_t>;

				const auto _min = (double)lim::min();
				const auto _max = (double)lim::max();

				const auto _rawRange = std::abs((double)random_engine::max() - (double)random_engine::min());
				const auto _raw =
					((double)this->random_uint() + (double)slim::min()) / _rawRange;

				const auto _lvl = std::lerp(_min, _max, _raw);
				return static_cast<T>(_lvl);
			};

		private:
			std::random_device dv;
			const std::seed_seq sseq{ dv(), dv(), dv(), dv(), dv(), dv(), dv() };
			std::mt19937 mt{ sseq };
		};
	
		inline random_generator& get_thread_random_generator()
		{
			static thread_local random_generator v{};
			return v;
		};
	};

#define _DEF_RANDOM_FN(type) template <> type random_t::invoke_impl() \
	{ \
		return impl::get_thread_random_generator().random<type>(); \
	}

	_DEF_RANDOM_FN(int8_t);
	_DEF_RANDOM_FN(int16_t);
	_DEF_RANDOM_FN(int32_t);
	_DEF_RANDOM_FN(int64_t);

	_DEF_RANDOM_FN(uint8_t);
	_DEF_RANDOM_FN(uint16_t);
	_DEF_RANDOM_FN(uint32_t);
	_DEF_RANDOM_FN(uint64_t);

	_DEF_RANDOM_FN(float);
	_DEF_RANDOM_FN(double);
	_DEF_RANDOM_FN(long double);
};