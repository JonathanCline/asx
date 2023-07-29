#pragma once

/** @file */

#include <string>
#include <string_view>

#if defined(WIN32) || defined(_WIN32)
	#define ASX_OS_WINDOWS
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
};