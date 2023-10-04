#pragma once

/** @file */

#include <asx/source.hpp>
#include <asx/format.hpp>

#include <string_view>

namespace asx
{
	/**
	 * @brief Levels defining the severity level a message needs to be at before being logged.
	*/
	enum class LogLevel
	{
		/**
		 * @brief Completely disables logging.
		*/
		none = 0,

		fatal,
		error,
		warn,
		info,

		/**
		 * @brief Enables all logging levels.
		*/
		all,
	};

	/**
	 * @brief Gets the current log level.
	 * @return Logging level value.
	*/
	LogLevel get_logging_level();

	/**
	 * @brief Sets the log level.
	 * @param _level Logging level value.
	*/
	void set_logging_level(LogLevel _level);


	/**
	 * @brief Flags that can be true/false to configure logging behavior.
	*/
	enum class LogFlag
	{
		/**
		 * @brief When true, emits ansi color codes when logging.
		 * 
		 * Defaults to false.
		*/
		use_ansi_colors = 1,
	};

	/**
	 * @brief Sets the state for one of the logging system's flags.
	 * @param _flag The log flag to change.
	 * @param _state The new state for the flag.
	*/
	void set_log_flag(LogFlag _flag, bool _state);

	/**
	 * @brief Duplicates logged messages to the given file, ansi colors will be ommitted (regardless of `LogFlag`s).
	 * 
	 * If this fails to open the given path, an error message is logged and the original file (if set)
	 * will remain as the destination file.
	 * 
	 * @param _path Absolute path to the file to write messages to.
	*/
	void set_log_file(const char* _path);

	/**
	 * @brief Closes the file previously set for logging, does nothing if none has been set.
	*/
	void close_log_file();

	/**
	 * @brief Checks if a file is currently being used for logging.
	*/
	bool has_log_file();


	/**
	 * @brief Writes a message to the log.
	 * @param _message The message to write.
	*/
	void append_log(std::string_view _message);


	/**
	 * @brief Writes a general info message to the log.
	 * @param _message The message to log.
	*/
	void log_info(std::string_view _message);

	/**
	 * @brief Writes a formatted general info message to the log.
	 * @param _fmt The formatting string for the message.
	 * @param _args... Formattable arguments.
	*/
	inline void log_info(std::string_view _fmt, const cx_formattable auto&... _args)
	{
		if (get_logging_level() >= LogLevel::info)
		{
			const auto s = asx::format(_fmt, _args...);
			log_info(s);
		};
	};
	

	/**
	 * @brief Writes a warning message to the log.
	 * @param _message The message to log.
	*/
	void log_warn(std::string_view _message);

	/**
	 * @brief Writes a formatted warning message to the log.
	 * @param _fmt The formatting string for the message.
	 * @param _args... Formattable arguments.
	*/
	inline void log_warn(std::string_view _fmt, const cx_formattable auto&... _args)
	{
		if (get_logging_level() >= LogLevel::warn)
		{
			const auto s = asx::format(_fmt, _args...);
			asx::log_warn(s);
		};
	};


	/**
	 * @brief Writes an error message to the log.
	 * @param _message The message to log.
	*/
	void log_error(std::string_view _message);

	/**
	 * @brief Writes an error message to the log, including source location.
	 * @param _trace Stack trace from the source of the error.
	 * @param _message The message to log.
	*/
	void log_error(const StackTraceView& _trace, std::string_view _message);

	/**
	 * @brief Writes an error message to the log, including source location.
	 * @param _trace Stack trace from the source of the error.
	 * @param _message The message to log.
	*/
	template <size_t N>
	inline void log_error(asx::BasicStackTrace<N>&& _trace, std::string_view _message)
	{
		if (get_logging_level() >= LogLevel::error)
		{
			const auto _traceView = asx::StackTraceView(_trace);
			log_error(_traceView, _message);
		};
	};

	/**
	 * @brief Writes a formatted error message to the log.
	 * @param _fmt The formatting string for the message.
	 * @param _args... Formattable arguments.
	*/
	inline void log_error(std::string_view _fmt, const cx_formattable auto&... _args)
	{
		if (get_logging_level() >= LogLevel::error)
		{
			const auto s = asx::format(_fmt, _args...);
			asx::log_error(s);
		};
	};

	/**
	 * @brief Writes a formatted error message to the log, including source location.
	 * @param _trace Stack trace from the source of the error.
	 * @param _fmt The formatting string for the message.
	 * @param _args... Formattable arguments.
	*/
	inline void log_error(const StackTraceView& _trace, std::string_view _fmt, const cx_formattable auto&... _args)
	{
		if (get_logging_level() >= LogLevel::error)
		{
			const auto s = asx::format(_fmt, _args...);
			asx::log_error(_trace, s);
		};
	};

	/**
	 * @brief Writes a formatted error message to the log, including source location.
	 * @param _trace Stack trace from the source of the error.
	 * @param _fmt The formatting string for the message.
	 * @param _args... Formattable arguments.
	*/
	template <size_t N>
	inline void log_error(asx::BasicStackTrace<N>&& _trace, std::string_view _fmt, const cx_formattable auto&... _args)
	{
		if (get_logging_level() >= LogLevel::error)
		{
			const auto _traceView = asx::StackTraceView(_trace);
			asx::log_error(_traceView, _fmt, _args...);
		};
	};






	/**
	 * @brief Writes a fatal error message to the log, including source location and a stack trace.
	 * @param _trace Stack trace from the source of the error.
	 * @param _message The message to log.
	*/
	void log_fatal_error(const asx::StackTraceView& _trace, std::string_view _message);

	/**
	 * @brief Writes a formatted fatal error message to the log, including source location and a stack trace.
	 * @param _trace Stack trace from the source of the error.
	 * @param _fmt The formatting string for the message.
	 * @param _args... Formattable arguments.
	*/
	inline void log_fatal_error(const asx::StackTraceView& _trace, std::string_view _fmt, const cx_formattable auto&... _args)
	{
		if (get_logging_level() >= LogLevel::fatal)
		{
			const auto s = asx::format(_fmt, _args...);
			asx::log_fatal_error(_trace, s);
		};
	};

	/**
	 * @brief Writes a fatal error message to the log, including source location and a stack trace.
	 * @param _trace Stack trace from the source of the error.
	 * @param _message The message to log.
	*/
	template <size_t N>
	inline void log_fatal_error(asx::BasicStackTrace<N>&& _trace, std::string_view _message)
	{
		if (get_logging_level() >= LogLevel::fatal)
		{
			const auto _traceView = asx::StackTraceView(_trace);
			asx::log_fatal_error(_traceView, _message);
		};
	};

	/**
	 * @brief Writes a formatted fatal error message to the log, including source location and a stack trace.
	 * @param _trace Stack trace from the source of the error.
	 * @param _fmt The formatting string for the message.
	 * @param _args... Formattable arguments.
	*/
	template <size_t N>
	inline void log_fatal_error(asx::BasicStackTrace<N>&& _trace, std::string_view _fmt, const cx_formattable auto&... _args)
	{
		if (get_logging_level() >= LogLevel::fatal)
		{
			const auto _traceView = asx::StackTraceView(_trace);
			asx::log_fatal_error(_traceView, _fmt, _args...);
		};
	};


};

#define ASX_LOG_INFO(fmt, ...) ::asx::log_info(fmt __VA_OPT__(,) __VA_ARGS__)
#define ASX_LOG_WARN(fmt, ...) ::asx::log_warn(fmt __VA_OPT__(,) __VA_ARGS__)
#define ASX_LOG_ERROR(fmt, ...) ::asx::log_error(::asx::get_stack_trace(), fmt __VA_OPT__(,) __VA_ARGS__)
#define ASX_LOG_FATAL(fmt, ...) ::asx::log_fatal_error(::asx::get_stack_trace(), fmt __VA_OPT__(,) __VA_ARGS__)

