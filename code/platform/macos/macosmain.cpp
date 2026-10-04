/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "windows.h"

extern int CALLBACK WinMain(HINSTANCE instance, HINSTANCE previous_instance, char * command_line, int command_show);


// WinMain reads the arguments through GetCommandLineW, so it is passed an empty command line.
int main(int, char **)
{
	char command_line[] = "";
	return(WinMain(nullptr, nullptr, command_line, SW_SHOWNORMAL));
}
