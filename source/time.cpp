#include <asx/time.hpp>
#include <asx/logging.hpp>

namespace asx
{
	utc_time get_utc_timestamp()
	{
		namespace ch = std::chrono;
#ifdef ASX_OS_WINDOWS
		return std::chrono::time_point_cast<utc_time::duration>(utc_clock::now());
#else
		// Convert into time_t
		const auto _timeT = ch::system_clock::to_time_t(ch::system_clock::now());

		// Convert into tm object
		tm _tm{};
		gmtime_r(&_timeT, &_tm);

		// Convert into duration
		utc_time_traits::rep d{};
		d += static_cast<utc_time_traits::rep>(_tm.tm_sec);  	   // seconds after the minute
		d += static_cast<utc_time_traits::rep>(_tm.tm_min) * 60;    // minutes after the hour
		d += static_cast<utc_time_traits::rep>(_tm.tm_hour) * 3600; // hours since midnight
		d += static_cast<utc_time_traits::rep>(_tm.tm_yday) * 24 * 3600; // days since year start
		d += (static_cast<utc_time_traits::rep>(_tm.tm_year) - 70) * 365 * 24 * 3600; // years since 1970
		const auto _dur = ch::duration_cast<utc_time_traits::duration>
		(
			std::chrono::seconds(d)
		);
		return utc_time(_dur);
#endif
	};

	local_time utc_timestamp_to_local_time(utc_time _utcTime)
	{
		namespace ch = std::chrono;
#ifdef ASX_OS_WINDOWS
		auto _systemTime = std::chrono::clock_cast<std::chrono::system_clock>(_time);
		auto _localTime = std::chrono::zoned_time(std::chrono::current_zone(), _systemTime);
		return _localTime;
#else
		// Assume time_t is seconds since epoch
		time_t _timer = static_cast<time_t>(_utcTime.time_since_epoch().count());
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