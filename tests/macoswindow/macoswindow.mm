/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include <cstdlib>
#include <cstring>

#include "macoswindow.h"

bool OpenTSMacOS_Test_Focus_Activation(void);
bool OpenTSMacOS_Test_Fullscreen_Window_Level(void);
bool OpenTSMacOS_Test_Fullscreen_Presentation(void);
bool OpenTSMacOS_Test_Request_Quit(void);
bool OpenTSMacOS_Test_Command_Q(void);
bool OpenTSMacOS_Test_Text_Input(void);
HWND TestFocus = nullptr;
HWND GetFocus(void) { return(TestFocus); }

LRESULT CALLBACK Windows_Procedure(HWND, UINT, WPARAM, LPARAM) { return(0); }
bool OpenTSMacOS_Is_Control_Window(HWND window) { return(window != nullptr && window == TestFocus); }
LRESULT OpenTSMacOS_Send_Control_Message(HWND, UINT, WPARAM, LPARAM) { return(0); }
LRESULT OpenTSMacOS_Dispatch_Control_Message(HWND, UINT, WPARAM, LPARAM) { return(0); }
LRESULT OpenTSMacOS_Def_Control_Message(HWND, UINT, WPARAM, LPARAM) { return(0); }
BOOL OpenTSMacOS_Get_Control_Rect(HWND, RECT *, BOOL) { return(FALSE); }
BOOL OpenTSMacOS_Control_Point_Transform(HWND, POINT *, BOOL) { return(FALSE); }
BOOL OpenTSMacOS_Invalidate_Control(HWND) { return(FALSE); }
HWND OpenTSMacOS_Set_Control_Focus(HWND) { return(nullptr); }
int OpenTSMacOS_Get_Control_Text(HWND, LPSTR, int) { return(0); }

namespace
{
	bool QuitCallbackReturned = false;

	void Reject_Immediate_Exit(void)
	{
		if (!QuitCallbackReturned) {
			_Exit(2);
		}
	}
}

int main(int argc, char ** argv)
{
	if (argc != 2) {
		return(2);
	}

	if (std::strcmp(argv[1], "text-input") == 0) {
		TestFocus = &TestFocus;
		return(OpenTSMacOS_Test_Text_Input() ? 0 : 1);
	}

	if (std::strcmp(argv[1], "focus") == 0) {
		return(OpenTSMacOS_Test_Focus_Activation() ? 0 : 1);
	}

	if (std::strcmp(argv[1], "quit") == 0) {
		std::atexit(Reject_Immediate_Exit);
		if (OpenTSMacOS_Test_Request_Quit()) {
			QuitCallbackReturned = true;
			OpenTSMacOS_Pump_Events();
		}
		QuitCallbackReturned = true;
		return(1);
	}

	if (std::strcmp(argv[1], "fullscreen-level") == 0) {
		return(OpenTSMacOS_Test_Fullscreen_Window_Level() ? 0 : 1);
	}

	if (std::strcmp(argv[1], "fullscreen-presentation") == 0) {
		return(OpenTSMacOS_Test_Fullscreen_Presentation() ? 0 : 1);
	}

	if (std::strcmp(argv[1], "command-q") == 0) {
		return(OpenTSMacOS_Test_Command_Q() ? 0 : 1);
	}

	return(2);
}
