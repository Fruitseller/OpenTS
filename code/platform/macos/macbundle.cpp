/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "macbundle.h"

#include <mach-o/dyld.h>
#include <pwd.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <vector>

namespace {

std::filesystem::path Executable_Path(void)
{
	uint32_t size = 0;
	_NSGetExecutablePath(nullptr, &size);
	if (size == 0) {
		return(std::filesystem::path());
	}

	std::vector<char> buffer(size);
	if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
		return(std::filesystem::path());
	}

	std::error_code error;
	std::filesystem::path const canonical = std::filesystem::weakly_canonical(buffer.data(), error);
	return(error ? std::filesystem::path(buffer.data()) : canonical);
}


// A bundle is recognized by the three directory names every macOS application bundle uses.
bool Are_Bundle_Directories(std::filesystem::path const & macos)
{
	return(macos.filename() == "MacOS"
		&& macos.parent_path().filename() == "Contents"
		&& macos.parent_path().parent_path().extension() == ".app");
}

} // namespace


namespace OpenTSMacOS {

bool Executable_Is_Bundled(void)
{
	std::filesystem::path const executable = Executable_Path();
	if (executable.empty()) {
		return(false);
	}

	return(Are_Bundle_Directories(executable.parent_path()));
}


std::string Bundle_Parent_Directory(void)
{
	std::filesystem::path const executable = Executable_Path();
	if (executable.empty()) {
		return(std::string());
	}

	std::filesystem::path const macos = executable.parent_path();
	if (!Are_Bundle_Directories(macos)) {
		return(std::string());
	}

	return(macos.parent_path().parent_path().parent_path().string());
}


std::string Bundle_Resources_Directory(void)
{
	std::filesystem::path const executable = Executable_Path();
	if (executable.empty()) {
		return(std::string());
	}

	std::filesystem::path const macos = executable.parent_path();
	if (!Are_Bundle_Directories(macos)) {
		return(std::string());
	}

	return((macos.parent_path() / "Resources").string());
}


std::string User_Data_Directory(void)
{
	char const * home = getenv("HOME");
	if (home == nullptr || *home == '\0') {
		if (passwd const * entry = getpwuid(getuid())) {
			home = entry->pw_dir;
		}
	}

	std::filesystem::path directory;
	if (home != nullptr && *home != '\0') {
		directory = std::filesystem::path(home) / "Library" / "Application Support" / "OpenTS";
	} else {
		std::error_code error;
		directory = std::filesystem::temp_directory_path(error);
		if (error) {
			directory = "/tmp";
		}
		directory /= "OpenTS";
	}

	std::error_code error;
	std::filesystem::create_directories(directory, error);
	return(directory.string());
}

} // namespace OpenTSMacOS
