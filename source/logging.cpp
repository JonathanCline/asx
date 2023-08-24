#include <asx/logging.hpp>

#include <asx/exclusive.hpp>

#include <span>
#include <mutex>
#include <string>
#include <fstream>
#include <ostream>
#include <sstream>
#include <iostream>
#include <optional>

namespace asx
{
	constexpr auto ERROR_COLOR_ANSI = "\x1b[38;5;9m";
	constexpr auto WARNING_COLOR_ANSI = "\x1b[0;33m";
	constexpr auto INFO_COLOR_ANSI = "\x1b[0;37m";
	
	constexpr auto RESET_COLOR_ANSI = "\x1b[0m";


	inline std::string stringify_stack_trace(const StackTraceView& _stack)
	{
		auto ss = std::stringstream{};
		auto n = 0;
		for (auto& v : _stack)
		{
			if (n++ != 0) { ss << '\n'; };
			ss << '\t' << v.function() << "() in " << v.file() << " line " << v.line();
		};
		return ss.str();
	};

	inline std::ostream& operator<<(std::ostream& _ostr, const SourceLocation& _source)
	{
		if (const auto f = _source.file(); !f.empty())
		{
			_ostr << f;
		}
		else
		{
			return _ostr;
		};

		if (const auto l = _source.line(); l != 0)
		{
			_ostr << " (line " << _source.line() << ')';
		};

		if (const auto f = _source.function(); !f.empty())
		{
			_ostr << " '" << f << '\'';
		};
		return _ostr;
	};


	struct osyncstream
	{
	public:
		using stream_type = std::ostream;
		using mutex_type = std::mutex;

		template <typename T>
		requires requires(stream_type& s, const T& v)
		{
			{ s << v } -> std::same_as<std::ostream&>;
		}
		friend inline osyncstream& operator<<(osyncstream& _ostr, const T& v)
		{
			auto p = _ostr.stream_.get();
			**p << v;
			return _ostr;
		};
	
		std::streambuf* rdbuf(std::streambuf* _buf) const
		{
			return this->stream_->get()->rdbuf(_buf);
		};

		auto stream()
		{
			return this->stream_.get();
		};

		explicit osyncstream(std::streambuf* _buf) :
			stream_(std::make_unique<stream_type>(_buf))
		{};

	private:
		asx::exclusive<std::unique_ptr<stream_type>, mutex_type> stream_;
	};

	struct LoggingSystem
	{
		std::mutex mtx_{};
		std::ofstream file_stream_;
		osyncstream stream_;
		LogLevel logging_level_ = LogLevel::all;
		bool ansi_colors_ = false;

		LoggingSystem() :
			stream_(std::cout.rdbuf())
		{};
	};

	inline LoggingSystem& logging_system()
	{
		static LoggingSystem _system{};
		return _system;
	};

	LogLevel get_logging_level()
	{
		auto& _system = logging_system();
		return _system.logging_level_;
	};

	void set_logging_level(LogLevel _level)
	{
		auto& _system = logging_system();
		_system.logging_level_ = _level;
	};



	void set_log_flag(LogFlag _flag, bool _state)
	{
		auto& _system = logging_system();
		switch (_flag)
		{
		case LogFlag::use_ansi_colors:
			_system.ansi_colors_ = _state;
			break;
		default:
			ASX_ASSERT(false && "unhandled log flag");
			break;
		};
	};

	void set_log_file(const char* _path)
	{
		auto _stream = std::ofstream(_path);
		if (!_stream.is_open())
		{
			ASX_LOG_ERROR("Failed to create/open logging file at path \"{}\"", _path);
			return;
		};

		auto& _system = logging_system();
		{
			const auto lck = std::unique_lock(_system.mtx_);
			_system.file_stream_ = std::move(_stream);
		};
		ASX_LOG_INFO("Set logging file path to \"{}\"", _path);
	};

	void close_log_file()
	{
		auto& _system = logging_system();
		{
			const auto lck = std::unique_lock(_system.mtx_);
			_system.file_stream_.close();
		};
	};

	bool has_log_file()
	{
		auto& _system = logging_system();
		const auto lck = std::unique_lock(_system.mtx_);
		return _system.file_stream_.is_open();
	};

	template <typename OT, typename T>
	inline OT& write(OT& _ostream, const T& _value)
	{
		_ostream << _value;
		return _ostream;
	};


	struct LogMessageParams
	{
		const char* ansi_color = RESET_COLOR_ANSI;
	};


	constexpr auto INFO_MESSAGE_PARAMS = LogMessageParams
	{
		.ansi_color = INFO_COLOR_ANSI
	};

	constexpr auto WARNING_MESSAGE_PARAMS = LogMessageParams
	{
		.ansi_color = WARNING_COLOR_ANSI
	};

	constexpr auto ERROR_MESSAGE_PARAMS = LogMessageParams
	{
		.ansi_color = ERROR_COLOR_ANSI
	};

	constexpr auto FATAL_ERROR_MESSAGE_PARAMS = LogMessageParams
	{
		.ansi_color = ERROR_COLOR_ANSI
	};




	inline void append_parts_log(const LogMessageParams& _params, const auto&... _parts)
	{
		auto& _system = logging_system();
		auto _lockedStream = _system.stream_.stream();
		auto& _stream = *_lockedStream.get()->get();

		auto& _fstream = _system.file_stream_;
		const auto lck = std::unique_lock(_system.mtx_);
		ASX_ASSERT(_system.file_stream_);

		const auto _useAnsiColors = _system.ansi_colors_;
		if (_useAnsiColors)
		{
			asx::write(_stream, _params.ansi_color);
		};

		(asx::write(_stream, _parts), ...);
		if (_fstream.is_open())
		{
			(asx::write(_fstream, _parts), ...);
			_fstream.flush();
		};

		if (_useAnsiColors)
		{
			asx::write(_stream, RESET_COLOR_ANSI);
		};
	};

	void append_log(const LogMessageParams& _params, std::string_view _message)
	{
		append_parts_log(_params, _message, '\n');
	};



	void log_info(std::string_view _message)
	{
		if (get_logging_level() >= LogLevel::info)
		{
			append_parts_log(INFO_MESSAGE_PARAMS, "[Info] ", _message, '\n');
		};
	};

	void log_warn(std::string_view _message)
	{
		if (get_logging_level() >= LogLevel::warn)
		{
			append_parts_log(WARNING_MESSAGE_PARAMS, "[Warning] ", _message, '\n');
		};
	};

	void log_error(std::string_view _message)
	{
		if (get_logging_level() >= LogLevel::error)
		{
			append_parts_log(ERROR_MESSAGE_PARAMS, "[Error] ", _message, '\n');
		};
	};
	void log_error(const StackTraceView& _trace, std::string_view _message)
	{
		if (get_logging_level() >= LogLevel::error)
		{
			append_parts_log(ERROR_MESSAGE_PARAMS, "[Error] ", _message, '\n', stringify_stack_trace(_trace), '\n');
		};
	};

	void log_fatal_error(const StackTraceView& _trace, std::string_view _message)
	{
		if (get_logging_level() >= LogLevel::fatal)
		{
			append_parts_log(FATAL_ERROR_MESSAGE_PARAMS, "[FATAL] ", _message, '\n', stringify_stack_trace(_trace), '\n');
		};
	};
};
