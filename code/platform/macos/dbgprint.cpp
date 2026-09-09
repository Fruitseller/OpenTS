/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#include "always.h"

#include "dbgprint.h"
#include "opents_build.h"
#include "windows.h"

#include <crt_externs.h>
#include <fnmatch.h>
#include <libproc.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <string>

namespace {

#ifdef _DEBUG
constexpr char BuildType[] = "debug";
#else
constexpr char BuildType[] = "release";
#endif

constexpr char DebugTruncationNotice[] =
	"\n*** Log size limit reached. Nothing further will be written to this file. ***\n";
constexpr size_t DebugMessageMax = 4096;
constexpr std::uintmax_t DebugLogMaxBytes = 64ULL * 1024ULL * 1024ULL;
constexpr std::uintmax_t DebugLogBudget = DebugLogMaxBytes - sizeof(DebugTruncationNotice) + 1;
constexpr unsigned DebugLogMaxAgeDays = 14;

// Global constructors log before this translation unit's globals are built,
// so the lock is constructed on first use.
std::recursive_mutex & Debug_Lock()
{
	static std::recursive_mutex lock;
	return lock;
}
bool DebugInitDone = false;
bool AtLineStart = true;
bool ConsoleActive = false;
FILE * DebugFile = nullptr;
char DebugFileName[MAX_PATH] = {};
char DebugDirectory[MAX_PATH] = {};
std::uintmax_t DebugBytesWritten = 0;

bool Command_Line_Requests_Console()
{
	int const argc = *_NSGetArgc();
	char const * const * argv = *_NSGetArgv();
	for (int index = 1; index < argc; ++index) {
		char const * token = argv[index];
		if (token == nullptr || token[0] != '-' || (token[1] != 'X' && token[1] != 'x')) {
			continue;
		}
		for (token += 2; *token != '\0'; ++token) {
			if (*token == 'C' || *token == 'c') {
				return true;
			}
		}
	}
	return false;
}

std::tm Local_Time(std::time_t stamp)
{
	std::tm result = {};
	localtime_r(&stamp, &result);
	return result;
}

void Write_Text_Locked(char const * text, size_t length)
{
	if (DebugFile != nullptr) {
		if (DebugBytesWritten + length > DebugLogBudget) {
			fwrite(DebugTruncationNotice, 1, sizeof(DebugTruncationNotice) - 1, DebugFile);
			fclose(DebugFile);
			DebugFile = nullptr;
		} else {
			DebugBytesWritten += fwrite(text, 1, length, DebugFile);
			fflush(DebugFile);
		}
	}

	if (ConsoleActive) {
		fwrite(text, 1, length, stderr);
		fflush(stderr);
	}
}

void Write_Message_Locked(char const * buffer, bool with_prefix)
{
	size_t const length = strlen(buffer);
	if (length == 0) {
		return;
	}

	if (with_prefix && AtLineStart) {
		auto const now = std::chrono::system_clock::now();
		auto const stamp = std::chrono::system_clock::to_time_t(now);
		std::tm const local = Local_Time(stamp);
		auto const milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()).count() % 1000;

		char stamped[DebugMessageMax + 32];
		int const written = snprintf(stamped, sizeof(stamped), "[%02d:%02d:%02d.%03lld] %s",
			local.tm_hour, local.tm_min, local.tm_sec, static_cast<long long>(milliseconds), buffer);
		if (written > 0) {
			Write_Text_Locked(stamped, std::min<size_t>(written, sizeof(stamped) - 1));
			AtLineStart = buffer[length - 1] == '\n';
			return;
		}
	}

	Write_Text_Locked(buffer, length);
	AtLineStart = buffer[length - 1] == '\n';
}

void Write_Banner_Locked(std::tm const & started)
{
	static char const Wordmark[] =
R"ART(  ___                  _____ ____
 / _ \ _ __   ___ _ __|_   _/ ___|
| | | | '_ \ / _ \ '_ \ | | \___ \
| |_| | |_) |  __/ | | || |  ___) |
 \___/| .__/ \___|_| |_||_| |____/
      |_|

)ART";
	Write_Message_Locked(Wordmark, false);

	char line[512];
	snprintf(line, sizeof(line), "Version  : OpenTS %s (%s build)\n", OPENTS_VERSION, BuildType);
	Write_Message_Locked(line, false);
	snprintf(line, sizeof(line), "Commit   : %s on %s%s\n", OPENTS_COMMIT, OPENTS_BRANCH,
		OPENTS_COMMIT_DIRTY ? " (modified)" : "");
	Write_Message_Locked(line, false);
	snprintf(line, sizeof(line), "Committed: %s\n", OPENTS_COMMIT_DATE);
	Write_Message_Locked(line, false);
	snprintf(line, sizeof(line), "Started  : %04d-%02d-%02d %02d:%02d:%02d\n",
		started.tm_year + 1900, started.tm_mon + 1, started.tm_mday,
		started.tm_hour, started.tm_min, started.tm_sec);
	Write_Message_Locked(line, false);

	struct utsname system = {};
	if (uname(&system) == 0) {
		snprintf(line, sizeof(line), "System   : %s %s (%s)\n", system.sysname, system.release, system.machine);
	} else {
		snprintf(line, sizeof(line), "System   : macOS\n");
	}
	Write_Message_Locked(line, false);

	char options[256] = "(none)";
	int const argc = *_NSGetArgc();
	char const * const * argv = *_NSGetArgv();
	size_t used = 0;
	for (int index = 1; index < argc; ++index) {
		int const written = snprintf(options + used, sizeof(options) - used, "%s%s",
			used == 0 ? "" : " ", argv[index]);
		if (written <= 0 || static_cast<size_t>(written) >= sizeof(options) - used) {
			break;
		}
		used += static_cast<size_t>(written);
	}
	snprintf(line, sizeof(line), "Options  : %s\n", options);
	Write_Message_Locked(line, false);
	Write_Message_Locked("--------------------------------------------------------------------------------\n", false);
}

void Init_Locked()
{
	if (DebugInitDone) {
		return;
	}
	DebugInitDone = true;
	ConsoleActive =
#ifdef _DEBUG
		true;
#else
		Command_Line_Requests_Console();
#endif

	char executable[PROC_PIDPATHINFO_MAXSIZE] = {};
	if (proc_pidpath(getpid(), executable, sizeof(executable)) > 0) {
		std::error_code error;
		std::filesystem::path directory = std::filesystem::weakly_canonical(executable, error).parent_path() / "Debug";
		std::filesystem::create_directories(directory, error);
		strncpy(DebugDirectory, directory.c_str(), sizeof(DebugDirectory) - 1);
		Delete_Files_Older_Than(directory.c_str(), "DEBUG_*.LOG", DebugLogMaxAgeDays);

		auto const stamp = std::time(nullptr);
		std::tm const local = Local_Time(stamp);
		char timestamp[32];
		strftime(timestamp, sizeof(timestamp), "%d-%m-%Y_%H-%M-%S", &local);
		std::filesystem::path log = directory / (std::string("DEBUG_") + timestamp + ".LOG");
		DebugFile = fopen(log.c_str(), "wx");
		if (DebugFile == nullptr) {
			log = directory / (std::string("DEBUG_") + timestamp + "_" + std::to_string(getpid()) + ".LOG");
			DebugFile = fopen(log.c_str(), "wx");
		}
		if (DebugFile != nullptr) {
			strncpy(DebugFileName, log.c_str(), sizeof(DebugFileName) - 1);
		}
	}

	auto const stamp = std::time(nullptr);
	Write_Banner_Locked(Local_Time(stamp));
}

void Emit(char const * buffer, bool with_prefix)
{
	std::lock_guard<std::recursive_mutex> lock(Debug_Lock());
	Init_Locked();
	Write_Message_Locked(buffer, with_prefix);
}

} // namespace

bool Delete_Files_Older_Than(char const * directory, char const * pattern, unsigned days)
{
	if (directory == nullptr || pattern == nullptr || days > 90) {
		return false;
	}

	std::error_code error;
	auto const cutoff = std::filesystem::file_time_type::clock::now() - std::chrono::hours(24 * days);
	std::filesystem::directory_iterator entries(directory, error);
	if (error) {
		return false;
	}

	for (auto const & entry : entries) {
		if (!entry.is_regular_file(error) || error || fnmatch(pattern, entry.path().filename().c_str(), 0) != 0) {
			error.clear();
			continue;
		}
		auto const written = entry.last_write_time(error);
		if (!error && written < cutoff) {
			std::filesystem::remove(entry.path(), error);
		}
		error.clear();
	}
	return true;
}

void Debug_Init()
{
	std::lock_guard<std::recursive_mutex> lock(Debug_Lock());
	Init_Locked();
}

void Debug_Init_Console()
{
	std::lock_guard<std::recursive_mutex> lock(Debug_Lock());
	Init_Locked();
	ConsoleActive = true;
}

void Debug_Console_Hold()
{
	if (ConsoleActive && isatty(STDIN_FILENO)) {
		DebugString("Press Return to close this window.\n");
		(void)getchar();
	}
}

char const * Debug_Log_File_Name()
{
	Debug_Init();
	return DebugFileName;
}

char const * Debug_Directory()
{
	Debug_Init();
	return DebugDirectory;
}

void __cdecl DebugString(char const * format, ...)
{
	int const saved_errno = errno;
	DWORD const saved_error = GetLastError();
	char buffer[DebugMessageMax];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(buffer, sizeof(buffer), format, arguments);
	va_end(arguments);
	Emit(buffer, true);
	errno = saved_errno;
	SetLastError(saved_error);
}

void __cdecl DebugStringNoPrefix(char const * format, ...)
{
	int const saved_errno = errno;
	DWORD const saved_error = GetLastError();
	char buffer[DebugMessageMax];
	va_list arguments;
	va_start(arguments, format);
	vsnprintf(buffer, sizeof(buffer), format, arguments);
	va_end(arguments);
	Emit(buffer, false);
	errno = saved_errno;
	SetLastError(saved_error);
}

char const * Last_Error_Text(unsigned long error)
{
	static thread_local char message[256];
	if (strerror_r(static_cast<int>(error), message, sizeof(message)) != 0) {
		message[0] = '\0';
	}
	return message;
}
