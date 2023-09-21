#pragma once

/** @file */

#include <asx/os.hpp>

#ifdef ASX_OS_WINDOWS

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>
#include <WinUser.h>
#include <shellapi.h>
#pragma comment(lib, "DbgHelp")
#endif

namespace asx
{
#ifdef ASX_OS_WINDOWS
	/**
	 * @brief OS Specific application data, kept hidden.
	*/
	struct OSApplicationData
	{
		HANDLE process = GetCurrentProcess();
		bool sym_init = false;

		OSApplicationData() = default;
	};
#else
	/**
	 * @brief OS Specific application data, kept hidden.
	*/
	struct OSApplicationData
	{
		OSApplicationData() = default;
	};
#endif

	OSApplicationData& os_application_data();

	std::string get_clipboard_text();

	std::string get_current_executable_path();
}