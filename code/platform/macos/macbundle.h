/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <string>

/*
 * The locations the game uses when its executable runs from an application bundle. A plain
 * executable reports no bundle and keeps the paths it has always used.
 */
namespace OpenTSMacOS {

bool Executable_Is_Bundled(void);

// The folder that holds the .app, which is the default game data directory for a bundle.
std::string Bundle_Parent_Directory(void);

// The bundle's Contents/Resources directory.
std::string Bundle_Resources_Directory(void);

// The player's data directory, created when possible. It falls back to the temporary
// directory when the account has no home directory.
std::string User_Data_Directory(void);

} // namespace OpenTSMacOS
