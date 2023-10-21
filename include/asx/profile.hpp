#pragma once

/** @file */

#include <asx/logging.hpp>

#include <chrono>
#include <string>
#include <vector>
#include <string_view>

namespace asx
{
	class ASXSingleTimeProfiler
	{
	public:
		using Clock = std::chrono::high_resolution_clock;
		using Duration = std::chrono::duration<double>;

		struct Lap
		{
			Duration time;
			std::string name;
		};
		struct Result
		{
			std::vector<Lap> laps;
			Duration total;
		};
		
		void lap(std::string_view _name)
		{
			const auto _now = Clock::now();
			const auto _elapsed = std::chrono::duration_cast<Duration>(_now - this->lap_start_time_);

			this->laps_.push_back(Lap{ _elapsed, std::string(_name) });

			const auto _postOtherStuffTime = Clock::now();
			const auto _otherFunctionElapsed = _postOtherStuffTime - _now;
			this->start_time_ += _otherFunctionElapsed;
			this->lap_start_time_ = _postOtherStuffTime;
		};

		Result result() const
		{
			return Result{ this->laps_, this->total_ };
		};

		void finish()
		{
			this->total_ = std::chrono::duration_cast<Duration>(Clock::now() - this->start_time_);
		};

		void print() const
		{
			std::string _fmt = "Time Profile Results :\n Total = {}\n ";
			for (auto& v : this->laps_)
			{
				// Defaults to seconds
				double _value = v.time.count();
				const char* _unit = "s";

				if (v.time.count() < 0.001)
				{
					// Microseconds
					_value *= 1'000'000.0;
					_unit = "us";
				}
				else if (v.time.count() < 1.0)
				{
					// Milliseconds
					_value *= 1'000.0;
					_unit = "ms";
				};

				_fmt.append(asx::format("{} = {} {}\n ", v.name, _value, _unit));
			};
			ASX_LOG_INFO(_fmt, this->total_);
		};

		void finish_and_print()
		{
			this->finish();
			this->print();
		};

		ASXSingleTimeProfiler() :
			start_time_{ Clock::now() },
			lap_start_time_{ this->start_time_ }
		{};
	private:
		Clock::time_point start_time_;
		Clock::time_point lap_start_time_;
		Duration total_{};
		std::vector<Lap> laps_{};
	};
};

#ifndef ASX_PROFILE_TIME_DISABLE
	#define ASX_PROFILE_TIME_START() ::asx::ASXSingleTimeProfiler _asxProfileTm{}
	#define ASX_PROFILE_TIME_LAP(lapName) { _asxProfileTm.lap(lapName); }
	#define ASX_PROFILE_TIME_FINISH() { _asxProfileTm.finish_and_print(); }
#else
	#define ASX_PROFILE_TIME_START() static_assert(true)
	#define ASX_PROFILE_TIME_LAP(lapName) static_assert(true)
	#define ASX_PROFILE_TIME_FINISH() static_assert(true)
#endif
