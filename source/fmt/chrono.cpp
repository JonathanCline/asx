#include <asx/fmt/chrono.hpp>

#ifndef ASX_OS_WINDOWS
    #include <ctime>
#endif

namespace asx
{
	std::string utc_timestamp_to_string(utc_time _time)
    {
        namespace ch = std::chrono;
        using namespace std::chrono_literals;

#ifdef ASX_OS_WINDOWS
        return asx::format("%D %T", _time);
#else
        const auto _timeSinceEpoch = _time.time_since_epoch();
        ch::seconds _timeSinceEpochSeconds = 0s;
        ch::nanoseconds _timeSinceEpochNanoseconds = 0ns;

        if constexpr (utc_time::period::num / utc_time::period::den >= 0)
        {
            // Time is in seconds or worse resolution (minutes, hours, etc..)
            _timeSinceEpochSeconds = ch::duration_cast<ch::seconds>(_timeSinceEpoch);
        }
        else
        {
            // Time has better resolution than seconds
            _timeSinceEpochSeconds = ch::duration_cast<ch::seconds>(_timeSinceEpoch);
            _timeSinceEpochNanoseconds =
                ch::duration_cast<ch::nanoseconds>(_timeSinceEpoch) -
                ch::duration_cast<ch::nanoseconds>(_timeSinceEpochSeconds);
        };

        // Convert our now split time into seconds/nanoseconds
        std::timespec _timespec{};
        _timespec.tv_sec = static_cast<time_t>(_timeSinceEpochSeconds.count());
        _timespec.tv_nsec = static_cast<time_t>(_timeSinceEpochNanoseconds.count());

        // Convert into utc time representation
        std::tm _tm{};
        gmtime_r(&_timespec.tv_sec, &_tm);

        // Format into a string
        auto _buffer = std::string(64, '\0');
        const auto _wroteCount = std::strftime(_buffer.data(), _buffer.size(), "%D %T", &_tm);

        // Trim string and return
        _buffer.resize(_wroteCount);
        return _buffer;
#endif
    };
};
