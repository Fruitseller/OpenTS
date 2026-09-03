/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "wincompat.h"

#include <cstdio>
#include <cstring>
#include <deque>
#include <vector>

// The native window layer is not linked into this test; the message queue
// below stands in for it so the modal loop can be driven deterministically.
namespace
{
	std::deque<MSG> TestQueue;
}

BOOL PeekMessage(MSG * message, HWND, UINT, UINT, UINT remove)
{
	if (TestQueue.empty()) return(FALSE);
	if (message != nullptr) *message = TestQueue.front();
	if (remove == PM_REMOVE) TestQueue.pop_front();
	return(TRUE);
}

BOOL PostMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	TestQueue.push_back({window, message, wparam, lparam, 0, {0, 0}});
	return(TRUE);
}

BOOL TranslateMessage(MSG const *) { return(TRUE); }

LRESULT DispatchMessage(MSG const * message)
{
	return(message ? OpenTSMacOS_Send_Control_Message(message->hwnd, message->message, message->wParam, message->lParam) : 0);
}

void PostQuitMessage(int exit_code) { PostMessage(nullptr, WM_QUIT, static_cast<WPARAM>(exit_code), 0); }

LRESULT SendMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	return(OpenTSMacOS_Send_Control_Message(window, message, wparam, lparam));
}

LRESULT DefWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	return(OpenTSMacOS_Def_Control_Message(window, message, wparam, lparam));
}

int GetWindowText(HWND window, char * text, int size) { return(OpenTSMacOS_Get_Control_Text(window, text, size)); }
BOOL GetClientRect(HWND window, RECT * rectangle) { return(OpenTSMacOS_Get_Control_Rect(window, rectangle, TRUE)); }
BOOL InvalidateRect(HWND, RECT const *, BOOL) { return(TRUE); }
int GetSystemMetrics(int) { return(0); }
BOOL OpenTSMacOS_Get_Native_Window_Rect(HWND, RECT *) { return(FALSE); }
BOOL OpenTSMacOS_Move_Native_Window(HWND, int, int, int, int, BOOL) { return(FALSE); }

namespace
{
	int Failures = 0;

	void Check(bool condition, char const * what)
	{
		std::printf("%-68s %s\n", what, condition ? "ok" : "FAILED");
		if (!condition) Failures++;
	}

	void Push_Word(std::vector<WORD> & words, WORD value) { words.push_back(value); }

	void Push_Dword(std::vector<WORD> & words, DWORD value)
	{
		words.push_back(static_cast<WORD>(value));
		words.push_back(static_cast<WORD>(value >> 16));
	}

	void Push_String(std::vector<WORD> & words, char const * text)
	{
		for (; *text != '\0'; ++text) words.push_back(static_cast<WORD>(*text));
		words.push_back(0);
	}

	void Align_Dword(std::vector<WORD> & words)
	{
		if (words.size() % 2 != 0) words.push_back(0);
	}

	// A classic template with a Button, a named-class trackbar, and the same
	// 200 by 100 dialog-unit frame as the measuring dialog.
	std::vector<WORD> Make_Classic_Template(void)
	{
		std::vector<WORD> words;
		Push_Dword(words, WS_CHILD | DS_SETFONT);
		Push_Dword(words, 0);
		Push_Word(words, 2);
		Push_Word(words, 0);
		Push_Word(words, 0);
		Push_Word(words, 200);
		Push_Word(words, 100);
		Push_Word(words, 0);
		Push_Word(words, 0);
		Push_String(words, "Title");
		Push_Word(words, 8);
		Push_String(words, "MS Sans Serif");

		Align_Dword(words);
		Push_Dword(words, WS_VISIBLE | WS_TABSTOP);
		Push_Dword(words, 0);
		Push_Word(words, 10);
		Push_Word(words, 20);
		Push_Word(words, 60);
		Push_Word(words, 14);
		Push_Word(words, 1);
		Push_Word(words, 0xffff);
		Push_Word(words, 0x0080);
		Push_String(words, "OK");
		Push_Word(words, 0);

		Align_Dword(words);
		Push_Dword(words, WS_VISIBLE);
		Push_Dword(words, 0);
		Push_Word(words, 0);
		Push_Word(words, 40);
		Push_Word(words, 100);
		Push_Word(words, 10);
		Push_Word(words, 2);
		Push_String(words, "msctls_trackbar32");
		Push_Word(words, 0);
		Push_Word(words, 0);
		return(words);
	}

	std::vector<WORD> Make_Extended_Template(void)
	{
		std::vector<WORD> words;
		Push_Word(words, 1);
		Push_Word(words, 0xffff);
		Push_Dword(words, 0);
		Push_Dword(words, 0);
		Push_Dword(words, WS_CHILD);
		Push_Word(words, 1);
		Push_Word(words, 0);
		Push_Word(words, 0);
		Push_Word(words, 100);
		Push_Word(words, 50);
		Push_Word(words, 0);
		Push_Word(words, 0);
		Push_Word(words, 0);

		Align_Dword(words);
		Push_Dword(words, 0);
		Push_Dword(words, 0);
		Push_Dword(words, WS_VISIBLE);
		Push_Word(words, 10);
		Push_Word(words, 10);
		Push_Word(words, 40);
		Push_Word(words, 12);
		Push_Dword(words, 300);
		Push_Word(words, 0xffff);
		Push_Word(words, 0x0081);
		Push_Word(words, 0);
		Push_Word(words, 0);
		return(words);
	}

	void Test_Classic_Template(void)
	{
		std::vector<WORD> const words = Make_Classic_Template();
		HWND const dialog = CreateDialogIndirectParam(nullptr,
			reinterpret_cast<LPCDLGTEMPLATE>(words.data()), nullptr, nullptr, 0);
		Check(dialog != nullptr, "classic template creates a dialog");

		RECT client = {};
		GetClientRect(dialog, &client);
		Check(client.right == 300 && client.bottom == 163, "dialog client area is 300 by 163 pixels");

		char text[64] = {};
		GetWindowText(dialog, text, sizeof(text));
		Check(std::strcmp(text, "Title") == 0, "dialog carries the template title");

		HWND const button = GetDlgItem(dialog, 1);
		char class_name[64] = {};
		GetClassName(button, class_name, sizeof(class_name));
		Check(std::strcmp(class_name, "Button") == 0, "ordinal 0x0080 creates a Button");
		GetWindowText(button, text, sizeof(text));
		Check(std::strcmp(text, "OK") == 0, "button carries the template text");
		Check((GetWindowLong(button, GWL_STYLE) & WS_TABSTOP) != 0, "button keeps the template style");

		RECT rectangle = {};
		GetWindowRect(button, &rectangle);
		Check(rectangle.left == 15 && rectangle.top == 33 && rectangle.right == 105 && rectangle.bottom == 56,
			"button rectangle uses 6 by 13 dialog-unit scaling");

		HWND const trackbar = GetDlgItem(dialog, 2);
		GetClassName(trackbar, class_name, sizeof(class_name));
		Check(std::strcmp(class_name, "msctls_trackbar32") == 0, "named control class is preserved");

		DestroyWindow(dialog);
	}

	void Test_Extended_Template(void)
	{
		std::vector<WORD> const words = Make_Extended_Template();
		HWND const dialog = CreateDialogIndirectParam(nullptr,
			reinterpret_cast<LPCDLGTEMPLATE>(words.data()), nullptr, nullptr, 0);
		Check(dialog != nullptr, "extended template creates a dialog");

		HWND const edit = GetDlgItem(dialog, 300);
		char class_name[64] = {};
		GetClassName(edit, class_name, sizeof(class_name));
		Check(std::strcmp(class_name, "Edit") == 0, "extended item ordinal 0x0081 creates an Edit");

		DestroyWindow(dialog);
	}

	HWND InitChild = nullptr;

	INT_PTR CALLBACK Init_Recording_Dialog_Proc(HWND window, UINT message, WPARAM, LPARAM)
	{
		if (message == WM_INITDIALOG) InitChild = GetDlgItem(window, 1);
		return(0);
	}

	void Test_Init_Order(void)
	{
		std::vector<WORD> const words = Make_Classic_Template();
		HWND const dialog = CreateDialogIndirectParam(nullptr,
			reinterpret_cast<LPCDLGTEMPLATE>(words.data()), nullptr, Init_Recording_Dialog_Proc, 0);
		Check(InitChild != nullptr, "children exist when WM_INITDIALOG arrives");
		DestroyWindow(dialog);
	}

	INT_PTR CALLBACK Zero_Result_Dialog_Proc(HWND window, UINT message, WPARAM, LPARAM)
	{
		if (message == WM_INITDIALOG) PostMessage(window, WM_APP, 0, 0);
		if (message == WM_APP) {
			EndDialog(window, 0);
			DestroyWindow(window);
		}
		return(0);
	}

	void Test_Modal_Result(void)
	{
		INT_PTR const result = DialogBoxParam(nullptr, MAKEINTRESOURCE(198), nullptr, Zero_Result_Dialog_Proc, 0);
		Check(result == 0, "modal loop returns the EndDialog result, including zero");
		Check(TestQueue.empty(), "modal loop consumed its messages");
	}

	INT_PTR CALLBACK Immediate_End_Dialog_Proc(HWND window, UINT message, WPARAM, LPARAM)
	{
		if (message == WM_INITDIALOG) EndDialog(window, 5);
		return(0);
	}

	void Test_Immediate_End(void)
	{
		INT_PTR const result = DialogBoxParam(nullptr, MAKEINTRESOURCE(198), nullptr, Immediate_End_Dialog_Proc, 0);
		Check(result == -1, "a dialog ended from WM_INITDIALOG terminates the loop");
	}

	void Test_Measuring_Fallback(void)
	{
		HWND const dialog = CreateDialogParam(nullptr, MAKEINTRESOURCE(198), nullptr, nullptr, 0);
		Check(dialog != nullptr, "template 198 falls back to the measuring dialog");
		RECT client = {};
		GetClientRect(dialog, &client);
		Check(client.right == 300 && client.bottom == 163, "measuring dialog client area is 300 by 163 pixels");
		DestroyWindow(dialog);

		Check(CreateDialogParam(nullptr, MAKEINTRESOURCE(9999), nullptr, nullptr, 0) == nullptr,
			"an unknown template creates no dialog");
	}
}

int main(void)
{
	Test_Classic_Template();
	Test_Extended_Template();
	Test_Init_Order();
	Test_Modal_Result();
	Test_Immediate_End();
	Test_Measuring_Fallback();

	std::printf("%s\n", Failures == 0 ? "all checks passed" : "checks FAILED");
	return(Failures == 0 ? 0 : 1);
}
