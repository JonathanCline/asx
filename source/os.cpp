#include "os.hpp"

#include <jclib/guard.h>
#include <jclib/unique.h>

#include <asx/assert.hpp>
#include <asx/logging.hpp>

#include <numeric>
#include <filesystem>

namespace asx
{
	OSApplicationData& os_application_data()
	{
		static OSApplicationData _data{};
		return _data;
	};

#ifdef ASX_OS_WINDOWS
	inline bool init_debugging(HANDLE _proc)
	{
		return SymInitialize(_proc, NULL, TRUE);
	};
	inline void finish_debugging(HANDLE _proc)
	{
		SymCleanup(_proc);
	};
#endif
};

namespace asx
{
	void os_application_init()
	{
#ifdef ASX_OS_WINDOWS
		auto& _data = os_application_data();
		if (const auto _result = init_debugging(_data.process); !_result)
		{
			ASX_LOG_ERROR("Failed to initialize WINAPI debug help {}", GetLastError());
		}
		else
		{
			_data.sym_init = _result;
		};
#endif
	};
	void os_application_cleanup()
	{
#ifdef ASX_OS_WINDOWS
		auto& _data = os_application_data();
		if (_data.sym_init)
		{
			_data.sym_init = false;
			finish_debugging(_data.process);
		};
#endif
	};
};


namespace asx
{
#ifdef ASX_OS_WINDOWS

	inline void close_clipboard_fn()
	{
		if (!CloseClipboard())
		{
			// TODO : Report error (?)
			ASX_LOG_ERROR("CloseClipboard() failed with error {}", GetLastError());
		};
	};

#endif

	std::string get_clipboard_text()
	{
		auto _text = std::string{};
#ifdef ASX_OS_WINDOWS
		if (!OpenClipboard(NULL))
		{
			ASX_LOG_ERROR("OpenClipboard() failed with error {}", GetLastError());
			return _text;
		};

		// Construct a jclib `fguard` to automatically close the clipboard on destruction
		const auto _clipboardGuard = jc::fguard<&close_clipboard_fn>();

		auto _clipboardData = static_cast<const char*>(GetClipboardData(CF_TEXT));
		if (_clipboardData)
		{
			_text = _clipboardData;
		};
#else
		ASX_LOG_WARN("get_clipboard_text was called but no implementation exists for the current platform");
#endif
		return _text;
	};

#ifdef ASX_OS_WINDOWS

	struct UniqueGlobalMemoryTraits
	{
		using value_type = HGLOBAL;
		static void reset(value_type&& _value)
		{
			if (GlobalFree(_value) != NULL)
			{
				// TODO : Report error (?)
				ASX_LOG_ERROR("Failed to preform GlobalFree() (error code {})", GetLastError());
			};
		};
		constexpr static bool good(const value_type& _value) noexcept
		{
			return _value != NULL;
		};
		constexpr static value_type null() noexcept
		{
			return NULL;
		};
	};

	using UniqueGlobalMemory = jc::unique_value<HGLOBAL, UniqueGlobalMemoryTraits>;

#endif



	void set_clipboard_text(const std::string_view _text)
	{
#ifdef ASX_OS_WINDOWS
		if (!OpenClipboard(NULL))
		{
			// TODO : Report error (?)
			ASX_LOG_ERROR("Failed to preform OpenClipboard() (error code {})", GetLastError());
			return;
		};

		// Construct a jclib `fguard` to automatically close the clipboard on destruction
		const auto _clipboardGuard = jc::fguard<&close_clipboard_fn>();
		
		// Empty out the clipboard
		if (!EmptyClipboard())
		{
			// TODO : Report error (?)
			ASX_LOG_ERROR("Failed to preform EmptyClipboard() (error code {})", GetLastError());
			return;
		};

		// not using new[] as this is *exactly* what windows expects us to do.
		// it thinks we are idiots i guess.
		//
		// its probably correct
		auto _textMem = UniqueGlobalMemory(GlobalAlloc(GMEM_MOVEABLE, _text.size() + 1));

		// Check if alloc failed.
		if (!_textMem)
		{
			// TODO : Report error (?)
			ASX_LOG_ERROR("Failed to preform GlobalAlloc() (error code {})", GetLastError());
			return;
		};

		// Lock the memory and copy data
		{
			auto _textCopy = static_cast<char*>(GlobalLock(_textMem.get()));
			if (!_textCopy)
			{
				// Failed to lock, return early.
				// TODO : Report error (?)
				ASX_LOG_ERROR("Failed to preform GlobalLock() (error code {})", GetLastError());
				return;
			};

			std::copy_n(_text.begin(), _text.size(), _textCopy);
			_textCopy[_text.size()] = '\0';
			
			// Ensure that no error code is currently set
			if (const auto _errorCode = GetLastError(); _errorCode != NO_ERROR)
			{
				// TODO : Report error (?)
				ASX_LOG_ERROR("Discarded WINAPI error code from unknown source (error code {})", GetLastError());
			};

			// Unlock the memory.
			if (const auto _unlockResult = GlobalUnlock(_textMem.get()); _unlockResult != 0)
			{
				// Memory is still locked, this should never occur.
				ASX_FAIL("A global memory handle was double locked when only a single lock was expected");
			}
			else
			{
				// Check if there is an error code set
				if (const auto _errorCode = GetLastError(); _errorCode != NO_ERROR)
				{
					// Some error occured.
					// TODO : Report error (?)
					ASX_LOG_ERROR("Failed to preform GlobalUnlock() (error code {})", GetLastError());
					return;
				};
			};
		};

		// Upload to clipboard and we are done!
		if (SetClipboardData(CF_TEXT, (HANDLE)(_textMem.get())) != _textMem.get())
		{
			// Error!
			// TODO : Report error (?)
			ASX_LOG_ERROR("Failed to perform SetClipboardData() (error code {})", GetLastError());
		};
#else
		ASX_LOG_WARN("set_clipboard_text was called but no implementation exists for the current platform");
#endif
	};

};

namespace asx
{
	std::string get_current_executable_path()
	{
#ifdef ASX_OS_WINDOWS
		// Buffer size clamped between 512 and 1024 bytes
		auto _buffer = std::array<char, std::clamp(MAX_PATH * 4, 512, 1024)> {};
		const auto _result = GetModuleFileNameA(NULL, _buffer.data(), static_cast<DWORD>(_buffer.size()));
		if (_result != 0)
		{
			// Return truncated string
			return std::string(_buffer.data(), static_cast<size_t>(_result));
		}
		else
		{
			// Error occured
			ASX_LOG_ERROR("Failed to perform GetModuleFileNameA() (error code {})", GetLastError());
			return std::string{};
		};
#elif defined(ASX_OS_LINUX)
		return std::filesystem::canonical("/proc/self/exe").string();
#else
#error "Not implemented for target system"
#endif
	};

	int get_system_dpi()
	{
#ifdef ASX_OS_WINDOWS
		return GetDpiForSystem();
#else
#error "Implement me!!!!"
#endif
	};

	bool open_file_path_in_file_explorer(const std::string& _path)
	{
#ifdef ASX_OS_WINDOWS
		auto _instance = ShellExecuteA(NULL, "open", _path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
		if ((INT_PTR)_instance <= 32)
		{
			// Failure
			ASX_LOG_ERROR("Failed to open file in file explorer");
			return false;
		}
		else
		{
			ASX_LOG_INFO("Opened path \"{}\" in system file explorer", _path);
			return true;
		};
#else
#error "Implement me!!!!"
#endif
	};
}