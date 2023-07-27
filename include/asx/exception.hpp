#pragma once

/** @File */

#include <asx/source.hpp>

#include <array>
#include <string>
#include <string_view>

namespace asx
{
	namespace impl
	{
		/**
		 * @brief Exception helper type carrying a source location.
		*/
		struct ExceptionWithSourceLocation
		{
		public:

			constexpr uint32_t line() const noexcept
			{
				return this->src_.line();
			};
			constexpr std::string_view file() const noexcept
			{
				return this->src_.file();
			};
			constexpr std::string_view function() const noexcept
			{
				return this->src_.function();
			};

			constexpr ExceptionWithSourceLocation() noexcept :
				src_{}
			{};
			constexpr ExceptionWithSourceLocation(SourceLocation _src) noexcept :
				src_(std::move(_src))
			{};
		private:
			SourceLocation src_;
		};

		/**
		 * @brief Exception helper type carrying a stack trace.
		*/
		struct ExceptionWithStackTrace : public std::exception
		{
		public:
			const StackTrace& stack_trace() const noexcept
			{
				return this->stack_;
			};
			ExceptionWithStackTrace() = default;
			explicit ExceptionWithStackTrace(StackTrace _stack) :
				std::exception(), stack_(std::move(_stack))
			{};
		private:
			StackTrace stack_;
		};
	};


	/**
	 * @brief Base type for astrum exceptions.
	*/
	class exception : public std::exception, impl::ExceptionWithSourceLocation
	{
	public:

		const char* what() const override
		{
			return this->what_.c_str();
		};

		exception() = default;

		explicit exception(std::string_view _message, const SourceLocation& _source) :
			impl::ExceptionWithSourceLocation(_source),
			what_(_message)
		{};
		explicit exception(std::string_view _message) :
			exception(_message, SourceLocation())
		{};

	private:
		SourceLocation src_;
		std::string what_;
	};

};

#define ASX_THROW(what) throw what

#define ASX_EXCEPTION(type, ...) type(__VA_ARGS__, ASX_HERE())
#define ASX_THROW_EXCEPTION(type, ...) ASX_THROW(ASX_EXCEPTION(type, __VA_ARGS__))
