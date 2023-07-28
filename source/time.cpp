#include <asx/time.hpp>

namespace asx
{
	utc_time get_utc_timestamp()
	{
		return std::chrono::time_point_cast<utc_time::duration>(utc_clock::now());
	};

	local_time utc_timestamp_to_local_time(utc_time _utcTime)
	{
		auto _systemTime = std::chrono::clock_cast<std::chrono::system_clock>(_utcTime);
		auto _localTime = std::chrono::zoned_time(std::chrono::current_zone(), _systemTime);
		return _localTime;
	};

};