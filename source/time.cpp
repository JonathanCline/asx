#include <asx/time.hpp>
#include <asx/logging.hpp>

namespace asx
{
	utc_time get_utc_timestamp()
	{
		namespace ch = std::chrono;
#ifdef ASX_OS_WINDOWS
		return std::chrono::time_point_cast<utc_time::duration>(std::chrono::utc_clock::now());
#else
		// Use timespec to get nanosecond (or at least better than full second) accuracy
		std::timespec _timespec{};
		const auto _result = std::timespec_get(&_timespec, TIME_UTC);
		const auto _duration =
			ch::duration_cast<utc_time_traits::duration>(ch::seconds(_timespec.tv_sec))
		 	+ ch::duration_cast<utc_time_traits::duration>(ch::nanoseconds(_timespec.tv_nsec));
		return utc_time(_duration);
#endif
	};

	local_time utc_timestamp_to_local_time(utc_time _utcTime)
	{
		namespace ch = std::chrono;
#ifdef ASX_OS_WINDOWS
		auto _systemTime = std::chrono::clock_cast<std::chrono::system_clock>(_utcTime);
		auto _localTime = std::chrono::zoned_time(std::chrono::current_zone(), _systemTime);
		return _localTime;
#else
		// Assume time_t is seconds since epoch
		time_t _timer = static_cast<time_t>
		(
			ch::duration_cast<ch::seconds>(
				_utcTime.time_since_epoch()
			).count()
		);
		tm _tm{};
		localtime_r(&_timer, &_tm);

		// Convert into duration
		local_time::rep d{};
		d += static_cast<local_time::rep>(_tm.tm_sec);  	   					 // seconds after the minute
		d += static_cast<local_time::rep>(_tm.tm_min) * 60;    					 // minutes after the hour
		d += static_cast<local_time::rep>(_tm.tm_hour) * 3600; 					 // hours since midnight
		d += static_cast<local_time::rep>(_tm.tm_yday) * 24 * 3600; 			 // days since year start
		d += (static_cast<local_time::rep>(_tm.tm_year) - 70) * 365 * 24 * 3600; // years since 1970
		const auto _dur = ch::duration_cast<local_time::duration>
		(
			std::chrono::seconds(d)
		);
		return local_time(_dur);
#endif
	};

};