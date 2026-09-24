/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "wincompat.h"

#include <algorithm>
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
	return(message ? OpenTSMacOS_Dispatch_Control_Message(message->hwnd, message->message, message->wParam, message->lParam) : 0);
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
BOOL InvalidateRect(HWND window, RECT const *, BOOL) { return(OpenTSMacOS_Invalidate_Control(window)); }
int GetSystemMetrics(int) { return(0); }
BOOL OpenTSMacOS_Get_Native_Window_Rect(HWND, RECT *) { return(FALSE); }
BOOL OpenTSMacOS_Move_Native_Window(HWND, int, int, int, int, BOOL) { return(FALSE); }
BOOL OpenTSMacOS_Client_To_Screen(HWND, POINT *) { return(TRUE); }
BOOL OpenTSMacOS_Screen_To_Client(HWND, POINT *) { return(TRUE); }
HWND OpenTSMacOS_Get_Main_Native_Window(void) { return(nullptr); }
BOOL ClientToScreen(HWND window, POINT * point) { return(OpenTSMacOS_Control_Point_Transform(window, point, TRUE)); }
BOOL ScreenToClient(HWND window, POINT * point) { return(OpenTSMacOS_Control_Point_Transform(window, point, FALSE)); }
HWND SetFocus(HWND window) { return(OpenTSMacOS_Set_Control_Focus(window)); }
SHORT GetAsyncKeyState(int) { return(0); }

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
		Check(std::all_of(TestQueue.begin(), TestQueue.end(), [](MSG const & message) {
			return(message.message == WM_PAINT && !OpenTSMacOS_Is_Control_Window(message.hwnd));
		}), "modal loop leaves only stale paints for destroyed controls");
		TestQueue.clear();
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

	void Test_Dialog_Hierarchy_And_Navigation(void)
	{
		HWND const dlg1 = CreateDialogParam(nullptr, MAKEINTRESOURCE(198), nullptr, nullptr, 0);
		HWND const dlg2 = CreateDialogParam(nullptr, MAKEINTRESOURCE(198), nullptr, nullptr, 0);

		Check(GetTopWindow(nullptr) == dlg1, "top window for null parent is the first dialog");
		Check(GetWindow(nullptr, GW_CHILD) == dlg1, "GW_CHILD of null parent is the first dialog");
		Check(GetWindow(dlg1, GW_HWNDNEXT) == dlg2, "GW_HWNDNEXT finds the sibling dialog");
		Check(IsChild(nullptr, dlg1), "IsChild with null parent recognizes dialog");

		BringWindowToTop(dlg2);
		Check(GetTopWindow(nullptr) == dlg2, "BringWindowToTop moves target window to top");
		Check(GetWindow(dlg2, GW_HWNDNEXT) == dlg1, "GW_HWNDNEXT reflects the updated order");

		SetForegroundWindow(dlg1);
		Check(GetTopWindow(nullptr) == dlg1, "SetForegroundWindow brings window to front");

		DestroyWindow(dlg1);
		DestroyWindow(dlg2);
		Check(GetTopWindow(nullptr) == nullptr, "all top-level dialogs removed on destroy");
	}

	struct DialogProcRecorder
	{
		static inline int LastCommandId = -1;
		static inline int CommandCount = 0;
		static inline int LastDrawItemAction = -1;
		static inline int LastCommandCheckState = -1;
		static inline HWND DependentCheckbox = nullptr;
		static inline int PaintCount = 0;
	};

	INT_PTR CALLBACK Recording_Dlg_Proc(HWND, UINT message, WPARAM wparam, LPARAM lparam)
	{
		if (message == WM_PAINT) {
			DialogProcRecorder::PaintCount++;
			return(TRUE);
		}
		if (message == WM_COMMAND) {
			DialogProcRecorder::LastCommandId = LOWORD(wparam);
			DialogProcRecorder::CommandCount++;
			if (lparam != 0) {
				DialogProcRecorder::LastCommandCheckState = static_cast<int>(SendMessage(reinterpret_cast<HWND>(lparam), BM_GETCHECK, 0, 0));
			}
			if (LOWORD(wparam) == 101 && DialogProcRecorder::DependentCheckbox != nullptr) {
				if (DialogProcRecorder::LastCommandCheckState == BST_CHECKED) {
					SendMessage(DialogProcRecorder::DependentCheckbox, BM_SETCHECK, BST_CHECKED, 0);
				}
			}
			return(TRUE);
		}
		if (message == WM_DRAWITEM) {
			DRAWITEMSTRUCT const * dis = reinterpret_cast<DRAWITEMSTRUCT const *>(lparam);
			if (dis) DialogProcRecorder::LastDrawItemAction = dis->itemAction;
			return(TRUE);
		}
		return(FALSE);
	}

	void Test_Button_Clicks_And_Commands(void)
	{
		DialogProcRecorder::LastCommandId = -1;
		DialogProcRecorder::CommandCount = 0;
		DialogProcRecorder::LastDrawItemAction = -1;
		DialogProcRecorder::LastCommandCheckState = -1;
		DialogProcRecorder::DependentCheckbox = nullptr;

		HWND const dialog = CreateDialogParam(nullptr, MAKEINTRESOURCE(198), nullptr, Recording_Dlg_Proc, 0);
		Check(dialog != nullptr, "recording dialog created");

		HWND const btn = CreateWindowEx(0, "Button", "TestButton", WS_VISIBLE | BS_OWNERDRAW,
			10, 10, 80, 24, dialog, reinterpret_cast<HMENU>(100), nullptr, nullptr);
		Check(btn != nullptr, "child button created");

		// Simulate left click down
		SendMessage(btn, WM_LBUTTONDOWN, 0, MAKELPARAM(5, 5));
		Check(DialogProcRecorder::LastDrawItemAction == ODA_SELECT, "LBUTTONDOWN sends WM_DRAWITEM with ODA_SELECT");

		// Simulate left click up inside rect
		SendMessage(btn, WM_LBUTTONUP, 0, MAKELPARAM(5, 5));
		Check(DialogProcRecorder::LastCommandId == 100, "LBUTTONUP inside button triggers WM_COMMAND with button ID");
		Check(DialogProcRecorder::CommandCount == 1, "button click counted once");

		// Checkbox test
		HWND const chk = CreateWindowEx(0, "Button", "Check", WS_VISIBLE | BS_AUTOCHECKBOX,
			10, 40, 80, 20, dialog, reinterpret_cast<HMENU>(101), nullptr, nullptr);
		Check(SendMessage(chk, BM_GETCHECK, 0, 0) == BST_UNCHECKED, "checkbox initial state is unchecked");

		HWND const chk_dep = CreateWindowEx(0, "Button", "Dependent", WS_VISIBLE | BS_AUTOCHECKBOX,
			10, 65, 80, 20, dialog, reinterpret_cast<HMENU>(102), nullptr, nullptr);
		DialogProcRecorder::DependentCheckbox = chk_dep;
		DialogProcRecorder::LastCommandCheckState = -1;

		SendMessage(chk, WM_LBUTTONDOWN, 0, MAKELPARAM(5, 5));
		SendMessage(chk, WM_LBUTTONUP, 0, MAKELPARAM(5, 5));
		Check(SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED, "checkbox toggles to checked on click");
		Check(DialogProcRecorder::LastCommandId == 101, "checkbox click sends WM_COMMAND to dialog");
		Check(DialogProcRecorder::LastCommandCheckState == BST_CHECKED, "WM_COMMAND handler reads toggled checkbox state");
		Check(SendMessage(chk_dep, BM_GETCHECK, 0, 0) == BST_CHECKED, "dependent checkbox checked via BM_SETCHECK inside command handler");

		DialogProcRecorder::DependentCheckbox = nullptr;

		DialogProcRecorder::LastDrawItemAction = -1;
		SendMessage(chk, WM_LBUTTONUP, 0, MAKELPARAM(5, 5));
		Check(DialogProcRecorder::LastDrawItemAction == -1, "checkbox release does not send an owner-draw button state");

		// Coordinates test
		SetWindowPos(dialog, nullptr, 100, 50, 300, 200, 0);
		SetWindowPos(btn, nullptr, 20, 30, 80, 24, 0);
		RECT btn_rect = {};
		GetWindowRect(btn, &btn_rect);
		Check(btn_rect.left == 120 && btn_rect.top == 80, "GetWindowRect calculates correct screen coordinates");

		POINT map_pt = {120, 80};
		MapWindowPoints(HWND_DESKTOP, dialog, &map_pt, 1);
		Check(map_pt.x == 20 && map_pt.y == 30, "MapWindowPoints maps screen to dialog coordinates correctly");

		// IsDialogMessage tests
		MSG esc_msg = {dialog, WM_KEYDOWN, VK_ESCAPE, 0, 0, {0, 0}};
		Check(IsDialogMessage(dialog, &esc_msg), "IsDialogMessage handles VK_ESCAPE");
		Check(DialogProcRecorder::LastCommandId == IDCANCEL, "VK_ESCAPE dispatches IDCANCEL command");

		MSG ret_msg = {dialog, WM_KEYDOWN, VK_RETURN, 0, 0, {0, 0}};
		Check(IsDialogMessage(dialog, &ret_msg), "IsDialogMessage handles VK_RETURN");
		Check(DialogProcRecorder::LastCommandId == IDOK, "VK_RETURN dispatches IDOK command");

		// ComboBox selection and dropped rect test
		HWND const combo = CreateWindowEx(0, "ComboBox", "Default", WS_VISIBLE | CBS_DROPDOWNLIST,
			10, 70, 100, 20, dialog, reinterpret_cast<HMENU>(102), nullptr, nullptr);
		SendMessage(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("GDI"));
		SendMessage(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>("Nod"));
		SendMessage(combo, CB_SETCURSEL, 1, 0);
		Check(SendMessage(combo, CB_GETCURSEL, 0, 0) == 1, "ComboBox CB_SETCURSEL selects index 1");

		char combo_text[32] = {};
		GetWindowText(combo, combo_text, sizeof(combo_text));
		Check(std::strcmp(combo_text, "Nod") == 0, "ComboBox GetWindowText returns selected item text");

		RECT drop_rect = {};
		SendMessage(combo, CB_GETDROPPEDCONTROLRECT, 0, reinterpret_cast<LPARAM>(&drop_rect));
		RECT expected_combo_rect = {};
		GetWindowRect(combo, &expected_combo_rect);
		Check(expected_combo_rect.bottom - expected_combo_rect.top == 24, "ComboBox closed height is 24");
		Check(drop_rect.left == expected_combo_rect.left && drop_rect.top == expected_combo_rect.top,
			"CB_GETDROPPEDCONTROLRECT returns screen coordinates matching GetWindowRect");

		HWND const combo2 = CreateWindowEx(0, "ComboBox", "Second", WS_VISIBLE | CBS_DROPDOWNLIST,
			10, 100, 100, 80, dialog, reinterpret_cast<HMENU>(103), nullptr, nullptr);
		RECT client_rect2 = {};
		GetClientRect(combo2, &client_rect2);
		Check(client_rect2.bottom - client_rect2.top == 24, "ComboBox GetClientRect returns closed height 24");
		RECT window_rect2 = {};
		GetWindowRect(combo2, &window_rect2);
		Check(window_rect2.bottom - window_rect2.top == 24, "ComboBox GetWindowRect returns closed height 24");
		RECT drop_rect2 = {};
		SendMessage(combo2, CB_GETDROPPEDCONTROLRECT, 0, reinterpret_cast<LPARAM>(&drop_rect2));
		Check(drop_rect2.bottom - drop_rect2.top == 80, "CB_GETDROPPEDCONTROLRECT returns dropped height 80");
		Check(expected_combo_rect.bottom <= window_rect2.top, "Vertically adjacent ComboBoxes do not overlap");

		SetWindowPos(combo2, nullptr, 10, 100, 100, 95, 0);
		GetWindowRect(combo2, &window_rect2);
		Check(window_rect2.bottom - window_rect2.top == 24, "SetWindowPos preserves ComboBox closed height 24");
		SendMessage(combo2, CB_GETDROPPEDCONTROLRECT, 0, reinterpret_cast<LPARAM>(&drop_rect2));
		Check(drop_rect2.bottom - drop_rect2.top == 95, "SetWindowPos updates dropped height to 95");

		// Child window destruction and parent invalidation
		HWND const child_win = CreateWindowEx(0, "Button", "Child", WS_VISIBLE, 5, 5, 20, 20, combo, nullptr, nullptr, nullptr);
		Check(DestroyWindow(child_win), "DestroyWindow on child window succeeds");

		// Hidden window redrawing
		HWND const hidden_dialog = CreateDialogParam(nullptr, MAKEINTRESOURCE(198), nullptr, Recording_Dlg_Proc, 0);
		ShowWindow(hidden_dialog, SW_HIDE);
		DialogProcRecorder::PaintCount = 0;
		SetWindowPos(hidden_dialog, nullptr, 0, 0, 100, 100, 0);
		RedrawWindow(hidden_dialog, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		Check(DialogProcRecorder::PaintCount == 0, "hidden dialog does not receive WM_PAINT on redraw or move");

		DialogProcRecorder::PaintCount = 0;
		ShowWindow(hidden_dialog, SW_SHOW);
		Check(DialogProcRecorder::PaintCount == 1, "showing dialog triggers a redraw");
		DialogProcRecorder::PaintCount = 0;
		RedrawWindow(hidden_dialog, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		Check(DialogProcRecorder::PaintCount == 1, "visible dialog receives WM_PAINT on RDW_UPDATENOW");
		DestroyWindow(hidden_dialog);

		DestroyWindow(dialog);
	}

	int PaintCount = 0;

	LRESULT CALLBACK Repainting_Control_Proc(HWND window, UINT message, WPARAM, LPARAM)
	{
		if (message == WM_PAINT) {
			++PaintCount;
			InvalidateRect(window, nullptr, FALSE);
			ValidateRect(window, nullptr);
		}
		return(0);
	}

	void Test_Paint_Queue_Drains(void)
	{
		TestQueue.clear();
		HWND const button = CreateWindowEx(0, "Button", "Cancel", WS_VISIBLE | BS_OWNERDRAW,
			0, 0, 80, 24, nullptr, nullptr, nullptr, nullptr);
		SetWindowLongPtr(button, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Repainting_Control_Proc));
		PaintCount = 0;
		InvalidateRect(button, nullptr, FALSE);
		InvalidateRect(button, nullptr, FALSE);
		for (int count = 0; count < 8 && !TestQueue.empty(); ++count) {
			MSG message = {};
			PeekMessage(&message, nullptr, 0, 0, PM_REMOVE);
			DispatchMessage(&message);
		}
		Check(TestQueue.empty(), "validated owner-draw repaint lets the message pump finish");
		Check(PaintCount == 1, "repeated invalidation produces one owner-draw repaint");
		Check(!GetUpdateRect(button, nullptr, FALSE), "validated control has no pending update region");
		SendMessage(button, WM_PAINT, 0, 0);
		Check(PaintCount == 2, "explicit synchronous paint still reaches a validated control");
		DestroyWindow(button);
		TestQueue.clear();
	}

	void Test_Edit_Input(void)
	{
		HWND const dialog = CreateDialogParam(nullptr, MAKEINTRESOURCE(198), nullptr, Recording_Dlg_Proc, 0);
		HWND const edit = CreateWindowEx(0, "Edit", "Player", WS_VISIBLE,
			10, 10, 100, 24, dialog, reinterpret_cast<HMENU>(104), nullptr, nullptr);
		SendMessage(edit, WM_LBUTTONDOWN, 0, MAKELPARAM(5, 5));
		SendMessage(edit, WM_LBUTTONUP, 0, MAKELPARAM(5, 5));
		Check(GetFocus() == edit, "clicking the name field gives it keyboard focus");
		SendMessage(edit, EM_SETSEL, 0, -1);
		SendMessage(edit, WM_CHAR, 'A', 0);
		SendMessage(edit, WM_CHAR, 'B', 0);
		char text[32] = {};
		GetWindowText(edit, text, sizeof(text));
		Check(std::strcmp(text, "AB") == 0, "typing replaces the selected player name");
		SendMessage(edit, WM_CHAR, VK_BACK, 0);
		GetWindowText(edit, text, sizeof(text));
		Check(std::strcmp(text, "A") == 0, "backspace removes a character from the name");
		SendMessage(edit, EM_SETLIMITTEXT, 2, 0);
		SendMessage(edit, WM_CHAR, 'B', 0);
		SendMessage(edit, WM_CHAR, 'C', 0);
		GetWindowText(edit, text, sizeof(text));
		Check(std::strcmp(text, "AB") == 0, "name input respects the configured text limit");
		SendMessage(edit, WM_KEYDOWN, VK_HOME, 0);
		SendMessage(edit, WM_KEYDOWN, VK_DELETE, 0);
		GetWindowText(edit, text, sizeof(text));
		Check(std::strcmp(text, "B") == 0, "Home and Delete edit the beginning of the name");
		MSG enter = {edit, WM_KEYDOWN, VK_RETURN, 0, 0, {0, 0}};
		IsDialogMessage(dialog, &enter);
		Check(DialogProcRecorder::LastCommandId == IDOK, "Enter in the name field invokes the dialog default button");
		DestroyWindow(dialog);
	}

	void Test_ZOrder_And_HitTesting(void)
	{
		HWND const dialog = CreateDialogParam(nullptr, MAKEINTRESOURCE(198), nullptr, nullptr, 0);
		Check(dialog != nullptr, "dialog created for zorder test");

		HWND const child1 = CreateWindowEx(0, "Button", "Child1", WS_VISIBLE, 10, 10, 50, 20, dialog, reinterpret_cast<HMENU>(1), nullptr, nullptr);
		HWND const child2 = CreateWindowEx(0, "Button", "Child2", WS_VISIBLE, 10, 40, 50, 20, dialog, reinterpret_cast<HMENU>(2), nullptr, nullptr);
		Check(GetTopWindow(dialog) == child1, "first created child is top of z-order");

		MoveWindow(child1, 15, 15, 50, 20, TRUE);
		MoveWindow(child2, 15, 45, 50, 20, TRUE);
		Check(GetTopWindow(dialog) == child1, "MoveWindow preserves child z-order");

		HWND const groupbox = CreateWindowEx(0, "Button", "Group", WS_VISIBLE | BS_GROUPBOX, 5, 5, 100, 100, dialog, reinterpret_cast<HMENU>(3), nullptr, nullptr);
		Check(SendMessage(groupbox, WM_NCHITTEST, 0, 0) == HTTRANSPARENT, "BS_GROUPBOX returns HTTRANSPARENT for WM_NCHITTEST");

		HWND const static_label = CreateWindowEx(0, "Static", "Label", WS_VISIBLE, 5, 110, 100, 20, dialog, reinterpret_cast<HMENU>(4), nullptr, nullptr);
		Check(SendMessage(static_label, WM_NCHITTEST, 0, 0) == HTTRANSPARENT, "non-notify Static returns HTTRANSPARENT for WM_NCHITTEST");

		Check(CallWindowProc(nullptr, child1, WM_NCHITTEST, 0, 0) == HTCLIENT, "CallWindowProc with null procedure falls back to DefWindowProc");

		DestroyWindow(dialog);
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
	Test_Dialog_Hierarchy_And_Navigation();
	Test_Button_Clicks_And_Commands();
	Test_ZOrder_And_HitTesting();
	Test_Edit_Input();
	Test_Paint_Queue_Drains();

	std::printf("%s\n", Failures == 0 ? "all checks passed" : "checks FAILED");
	return(Failures == 0 ? 0 : 1);
}
