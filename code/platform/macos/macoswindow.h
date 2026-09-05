/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <windows.h>

HWND OpenTSMacOS_Create_Window(int width, int height, bool windowed);
void OpenTSMacOS_Destroy_Window(HWND window);
void * OpenTSMacOS_Native_Window_Handle(HWND window);
HWND OpenTSMacOS_Get_Main_Native_Window(void);

void OpenTSMacOS_Pump_Events(void);

BOOL OpenTSMacOS_Get_Client_Rect(HWND window, RECT * rectangle);
BOOL OpenTSMacOS_Get_Window_Rect(HWND window, RECT * rectangle);
BOOL OpenTSMacOS_Client_To_Screen(HWND window, POINT * point);
BOOL OpenTSMacOS_Screen_To_Client(HWND window, POINT * point);
BOOL OpenTSMacOS_Get_Cursor_Pos(POINT * point);
BOOL OpenTSMacOS_Set_Cursor_Pos(int x, int y);
BOOL OpenTSMacOS_Clip_Cursor(RECT const * rectangle);
int OpenTSMacOS_Show_Cursor(BOOL show);

HCURSOR OpenTSMacOS_Create_Cursor(void const * rgba_pixels, int width, int height,
	int hot_x, int hot_y);
void OpenTSMacOS_Destroy_Cursor(HCURSOR cursor);
void OpenTSMacOS_Select_Cursor(HCURSOR cursor, bool visible);

int OpenTSMacOS_Display_Refresh_Rate(HWND window);
int * OpenTSMacOS_Enumerate_Display_Modes(HWND window, int min_width, int min_height,
	int max_width, int max_height);
