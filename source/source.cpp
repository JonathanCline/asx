#include <asx/source.hpp>

#include "os.hpp"
#include <asx/assert.hpp>
#include <asx/logging.hpp>

#include <span>
#include <mutex>

#ifdef ASX_OS_WINDOWS

namespace asx
{
	/**
	 * @brief Acquires a lock on a mutex used to enforce sequential access to the WINAPI functions used for the stack tracing.
	 * @return Owning lock handle.
	*/
	inline auto acquire_stack_trace_function_lock()
	{
		static std::mutex mtx{};
		return std::unique_lock(mtx);
	};

	inline std::string get_winapi_error_message(DWORD _errorCode)
	{
		const DWORD _flags =
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_ALLOCATE_BUFFER;
		
		char* _buffer = nullptr;
		const auto _bufferLen = FormatMessageA
		(
			_flags,
			nullptr,
			_errorCode, 0,
			reinterpret_cast<char*>(&_buffer), 0,
			nullptr
		);

		// String to write data into or return on failure
		auto _message = std::string();

		// Copy buffer data on success
		if (_bufferLen != 0)
		{
			_message = std::string(_buffer, _bufferLen);
		};

		// Cleanup buffer if it was allocated
		if (_buffer)
		{
			LocalFree(_buffer);
		};

		return _message;
	};


	size_t get_stack_trace(std::span<SourceLocation> _outBuffer, size_t _skipFrames)
	{
		// Check if debugging is enabled
		{
			const auto& _appData = asx::os_application_data();
			if (!_appData.sym_init)
			{
				// Exit early as sym hasn't been initialized.
				return 0;
			};
		};

		// Stores the stack frame addresses.
		auto _stackAddressStorage = std::make_unique<void*[]>(_outBuffer.size() + _skipFrames);

		// The current process.
		HANDLE _process = GetCurrentProcess();

		// How many stack addresses were actually read.
		size_t _stackAddressSize = 0;
		{
			const auto lck = acquire_stack_trace_function_lock();
			_stackAddressSize = static_cast<size_t>(
				CaptureStackBackTrace(1, (DWORD)_outBuffer.size(), _stackAddressStorage.get(), NULL)
			);
		};

		// Span viewing the stack addresses that were actually read.
		const auto _stackAddresses = std::span<void* const>
		(
			_stackAddressStorage.get(), _stackAddressSize
		);

		char buf[sizeof(SYMBOL_INFO) + (SourceLocation::function_name_max_size() - 1) * sizeof(TCHAR)];
		SYMBOL_INFO* symbol = (SYMBOL_INFO*)buf;
		symbol->MaxNameLen = SourceLocation::function_name_max_size();
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		DWORD displacement{};
		IMAGEHLP_LINE64 line{};
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		size_t _outIndex = 0;

		// Loop frames until we reach main
		for (size_t i = 0; i != _stackAddresses.size(); i++)
		{
			// Alias to the frame we are writing to.
			auto& _frame = _outBuffer[_outIndex];

			// Exit early if we hit the end of the output buffer.
			if (_outIndex == _outBuffer.size())
			{
				break;
			};

			// The address for this stack frame.
			DWORD64 address = reinterpret_cast<DWORD64>(_stackAddresses[i]);

			{
				auto lck = acquire_stack_trace_function_lock();
				if (!SymFromAddr(_process, address, NULL, symbol))
				{
					const auto _err = GetLastError();

					if (_err == 126)
					{
						// Error 126 can be ignored as it just means the module
						// where a symbol exists is outside of where we can look
						// or doesnt have the debug info we need
						//
						// We can report this part of the stack trace as just
						// external module for now
						_frame = SourceLocation::from_absolute_path
						(
							std::string_view(),
							std::string_view(),
							0
						);

						// Increment our out index so we write to the next frame.
						++_outIndex;
					}
					else
					{
						asx::log_error(
							asx::format(
								"SymFromAddr failed with error {} - \"{}\"",
								_err,
								get_winapi_error_message(_err)
							));
					};

					// Skip to next address in the stack
					continue;
				};
			};

			// Grab the function name string
			const auto _functionName = std::string_view(symbol->Name, symbol->NameLen);

			// True if this stack frame is the main function.
			const auto _isMainFunction = (_functionName == "main");

			// Check if we are still skipping frames
			if (i < _skipFrames)
			{
				// If we hit the main function, exit early.
				// Otherwise just skip the frame.
				if (_isMainFunction)
				{
					break;
				}
				else
				{
					continue;
				};
			};

			{
				auto lck = acquire_stack_trace_function_lock();
				if (SymGetLineFromAddr64(_process, address, &displacement, &line))
				{
					// Remove project root path from the file name.
					const auto _fileName = fix_project_source_file_path(
						std::string_view(line.FileName, std::strlen(line.FileName))
					);

					// The line number
					const auto& _lineNumber = line.LineNumber;

					// Write the frame with function name, file name, and line number
					_frame = SourceLocation::from_absolute_path
					(
						_fileName,
						_functionName,
						_lineNumber
					);
				}
				else
				{
					// Write the frame with only the function name.
					_frame = SourceLocation
					(
						std::string_view(),
						_functionName,
						0
					);
				};
			};

			// Increment our out index so we write to the next frame.
			++_outIndex;

			// Exit if we made it back to the application entry point.
			// This prevents reading from frames outside of the project source code.
			if (_isMainFunction)
			{
				break;
			};
		};

		return _outIndex;
	};
};

#else

namespace asx
{
	size_t get_stack_trace(std::span<StackFrame> _outBuffer)
	{
		ASX_LOG_WARN("Called get_stack_trace() but no implementation exists for the current platform.");
		return 0;
	};
};

#endif
