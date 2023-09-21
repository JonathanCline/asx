#pragma once

/** @file */

#include <string>
#include <string_view>

#if defined(WIN32) || defined(_WIN32)
	#define ASX_OS_WINDOWS
#elif defined(__linux__)
	#define ASX_OS_LINUX
#else
#error Finish me
#endif



namespace asx
{
	/**
	 * @brief Initalization function that (may) preform OS specific functionality.
	*/
	void os_application_init();

	/**
	 * @brief Cleanup function that (may) preform OS specific functionality.
	*/
	void os_application_cleanup();

	/**
	 * @brief Automatically runs the application cleanup function on destruction.
	*/
	struct OSApplicationCleanupGuard final
	{
		explicit OSApplicationCleanupGuard() = default;
		OSApplicationCleanupGuard(const OSApplicationCleanupGuard&) = delete;
		OSApplicationCleanupGuard& operator=(const OSApplicationCleanupGuard&) = delete;
		OSApplicationCleanupGuard(OSApplicationCleanupGuard&&) = delete;
		OSApplicationCleanupGuard& operator=(OSApplicationCleanupGuard&&) = delete;
		~OSApplicationCleanupGuard()
		{
			asx::os_application_cleanup();
		};
	};
};

namespace asx
{
	/**
	 * @brief Retrieves the text from the clipboard.
	 * @return Text from clipboard, or empty if none is there.
	*/
	std::string get_clipboard_text();

	/**
	 * @brief Sets the clipboard's data to hold some text.
	 * @param _text ascii string.
	*/
	void set_clipboard_text(const std::string_view _text);

	/**
	 * @brief Gets the path to the running executable.
	 * @return File path to running executable, or an empty string if something went wrong.
	*/
	std::string get_current_executable_path();

	/**
	 * @brief Gets the DPI for the current system.
	 * @return DPI value on success, -1 on error.
	*/
	int get_system_dpi();

	/**
	 * @brief Attempts to open a file path in the user's file explorer.
	 * @param _path File path.
	 * @return True on good open, false otherwise.
	*/
	bool open_file_path_in_file_explorer(const std::string& _path);
};