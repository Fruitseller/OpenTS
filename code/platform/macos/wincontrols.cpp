/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "wincompat.h"
#include "commctrl.h"

#include <memory>

namespace
{
	struct ControlItem
	{
		std::string Text;
		LPARAM Data = 0;
		bool Selected = false;
	};

	struct ControlWindow
	{
		std::string ClassName = "STATIC";
		std::string Text;
		RECT Rectangle = {0, 0, 100, 24};
		LONG_PTR Style = WS_VISIBLE;
		LONG_PTR ExtendedStyle = 0;
		LONG_PTR Identifier = 0;
		LONG_PTR UserData = 0;
		HWND Parent = nullptr;
		WNDPROC Procedure = DefWindowProc;
		DLGPROC DialogProcedure = nullptr;
		bool Enabled = true;
		bool Visible = true;
		bool DropdownVisible = false;
		int CurrentSelection = -1;
		int TopIndex = 0;
		int ItemHeight = 16;
		int CheckState = BST_UNCHECKED;
		int RangeMinimum = 0;
		int RangeMaximum = 100;
		int Position = 0;
		std::size_t TextLimit = 0x7fffffff;
		std::vector<ControlItem> Items;
		std::vector<HWND> Children;
		std::unordered_map<int, LONG_PTR> ExtraValues;
	};

	std::recursive_mutex ControlMutex;
	std::unordered_map<HWND, std::unique_ptr<ControlWindow>> Controls;
	std::unordered_map<std::string, WNDPROC> Classes;
	std::unordered_map<std::uint64_t, std::shared_ptr<std::atomic<bool>>> Timers;
	HWND FocusWindow = nullptr;
	HWND CaptureWindow = nullptr;

	ControlWindow * Find_Control(HWND window)
	{
		auto const found = Controls.find(window);
		return(found == Controls.end() ? nullptr : found->second.get());
	}

	HWND Add_Control(std::unique_ptr<ControlWindow> window)
	{
		HWND const handle = window.get();
		if (ControlWindow * parent = Find_Control(window->Parent)) {
			parent->Children.push_back(handle);
		}
		Controls.emplace(handle, std::move(window));
		return(handle);
	}

	void Infer_Control_Class(ControlWindow & control, UINT message)
	{
		if (message >= CB_GETEDITSEL && message <= CB_GETTOPINDEX) control.ClassName = "COMBOBOX";
		else if (message >= LB_ADDSTRING && message <= LB_FINDSTRINGEXACT) control.ClassName = "LISTBOX";
		else if (message >= BM_GETCHECK && message <= BM_CLICK) control.ClassName = "BUTTON";
		else if (message >= EM_SETSEL && message <= EM_SETLIMITTEXT) control.ClassName = "EDIT";
		else if (message == TBM_SETPOS || message == TBM_SETRANGE) control.ClassName = "msctls_trackbar32";
	}

	LRESULT Insert_Item(ControlWindow & control, int index, char const * text)
	{
		if (index < 0 || index > static_cast<int>(control.Items.size())) index = static_cast<int>(control.Items.size());
		ControlItem item;
		item.Text = text ? text : "";
		control.Items.insert(control.Items.begin() + index, std::move(item));
		if (control.CurrentSelection >= index) ++control.CurrentSelection;
		return(index);
	}

	LRESULT Base_Control_Message(ControlWindow & control, UINT message, WPARAM wparam, LPARAM lparam)
	{
		Infer_Control_Class(control, message);
		if (message == WM_USER + 1) {
			if (control.ClassName == "msctls_trackbar32") return(control.RangeMinimum);
			if (lparam != 0) {
				control.ClassName = "msctls_progress32";
				control.RangeMinimum = static_cast<SHORT>(LOWORD(lparam));
				control.RangeMaximum = static_cast<SHORT>(HIWORD(lparam));
				return(0);
			}
			control.Position = static_cast<int>(wparam);
			return(0);
		}
		if (message == WM_USER + 2) {
			if (control.ClassName == "msctls_trackbar32") return(control.RangeMaximum);
			if (control.ClassName == "msctls_progress32") {
				int const previous = control.Position;
				control.Position = static_cast<int>(wparam);
				return(previous);
			}
			return(control.Position);
		}
		switch (message) {
			case WM_SETTEXT:
				control.Text.assign(reinterpret_cast<char const *>(lparam) ? reinterpret_cast<char const *>(lparam) : "", 0, control.TextLimit);
				return(TRUE);
			case WM_GETTEXT:
				if (lparam == 0 || wparam == 0) return(0);
				std::strncpy(reinterpret_cast<char *>(lparam), control.Text.c_str(), wparam - 1);
				reinterpret_cast<char *>(lparam)[wparam - 1] = '\0';
				return(std::min<std::size_t>(control.Text.size(), wparam - 1));
			case WM_GETTEXTLENGTH: return(control.Text.size());
			case BM_GETCHECK: return(control.CheckState);
			case BM_SETCHECK: control.CheckState = static_cast<int>(wparam); return(0);
			case EM_SETLIMITTEXT: control.TextLimit = wparam; return(0);
			case CB_ADDSTRING:
			case LB_ADDSTRING: return(Insert_Item(control, -1, reinterpret_cast<char const *>(lparam)));
			case CB_INSERTSTRING:
			case LB_INSERTSTRING: return(Insert_Item(control, static_cast<int>(wparam), reinterpret_cast<char const *>(lparam)));
			case CB_DELETESTRING:
			case LB_DELETESTRING:
				if (wparam >= control.Items.size()) return(CB_ERR);
				control.Items.erase(control.Items.begin() + wparam);
				if (control.CurrentSelection == static_cast<int>(wparam)) control.CurrentSelection = -1;
				return(control.Items.size());
			case CB_RESETCONTENT:
			case LB_RESETCONTENT: control.Items.clear(); control.CurrentSelection = -1; control.TopIndex = 0; return(0);
			case CB_GETCOUNT:
			case LB_GETCOUNT: return(control.Items.size());
			case CB_GETCURSEL:
			case LB_GETCURSEL: return(control.CurrentSelection < 0 ? CB_ERR : control.CurrentSelection);
			case CB_SETCURSEL:
			case LB_SETCURSEL:
				if (static_cast<INT_PTR>(wparam) < -1 || wparam >= control.Items.size()) return(CB_ERR);
				control.CurrentSelection = static_cast<int>(wparam);
				return(control.CurrentSelection);
			case CB_GETLBTEXT:
			case LB_GETTEXT:
				if (wparam >= control.Items.size() || lparam == 0) return(CB_ERR);
				std::strcpy(reinterpret_cast<char *>(lparam), control.Items[wparam].Text.c_str());
				return(control.Items[wparam].Text.size());
			case CB_GETITEMDATA:
			case LB_GETITEMDATA: return(wparam < control.Items.size() ? control.Items[wparam].Data : CB_ERR);
			case CB_SETITEMDATA:
			case LB_SETITEMDATA:
				if (wparam >= control.Items.size()) return(CB_ERR);
				control.Items[wparam].Data = lparam;
				return(0);
			case CB_FINDSTRING:
			case CB_FINDSTRINGEXACT:
			case LB_FINDSTRING:
			case LB_FINDSTRINGEXACT: {
				char const * needle = reinterpret_cast<char const *>(lparam);
				if (!needle) return(CB_ERR);
				for (std::size_t index = 0; index < control.Items.size(); ++index) {
					bool const exact = message == CB_FINDSTRINGEXACT || message == LB_FINDSTRINGEXACT;
					if ((exact && strcasecmp(control.Items[index].Text.c_str(), needle) == 0)
					|| (!exact && strncasecmp(control.Items[index].Text.c_str(), needle, std::strlen(needle)) == 0)) return(index);
				}
				return(CB_ERR);
			}
			case CB_GETITEMHEIGHT:
			case LB_GETITEMHEIGHT: return(control.ItemHeight);
			case CB_SETITEMHEIGHT:
			case LB_SETITEMHEIGHT: control.ItemHeight = static_cast<int>(lparam); return(0);
			case CB_GETTOPINDEX:
			case LB_GETTOPINDEX: return(control.TopIndex);
			case CB_SETTOPINDEX:
			case LB_SETTOPINDEX: control.TopIndex = std::max(0, static_cast<int>(wparam)); return(0);
			case CB_SHOWDROPDOWN: control.DropdownVisible = wparam != 0; return(TRUE);
			case CB_GETDROPPEDSTATE: return(control.DropdownVisible);
			case CB_GETDROPPEDCONTROLRECT:
				if (lparam) *reinterpret_cast<RECT *>(lparam) = control.Rectangle;
				return(TRUE);
			case LB_GETSEL:
				return(wparam < control.Items.size() ? control.Items[wparam].Selected : LB_ERR);
			case LB_SETSEL:
				if (static_cast<INT_PTR>(lparam) == -1) {
					for (auto & item : control.Items) item.Selected = wparam != 0;
					return(0);
				}
				if (lparam < 0 || static_cast<std::size_t>(lparam) >= control.Items.size()) return(LB_ERR);
				control.Items[lparam].Selected = wparam != 0;
				return(0);
			case LB_GETSELCOUNT:
				return(std::count_if(control.Items.begin(), control.Items.end(), [](ControlItem const & item) { return(item.Selected); }));
			case LB_GETSELITEMS: {
				int * indexes = reinterpret_cast<int *>(lparam);
				int count = 0;
				for (std::size_t index = 0; indexes && index < control.Items.size() && count < static_cast<int>(wparam); ++index) {
					if (control.Items[index].Selected) indexes[count++] = static_cast<int>(index);
				}
				return(count);
			}
			case LB_SELITEMRANGE: {
				int const first = LOWORD(lparam);
				int const last = HIWORD(lparam);
				for (int index = first; index <= last && index < static_cast<int>(control.Items.size()); ++index) {
					if (index >= 0) control.Items[index].Selected = wparam != 0;
				}
				return(0);
			}
			case LB_GETITEMRECT:
				if (wparam >= control.Items.size() || !lparam) return(LB_ERR);
				*reinterpret_cast<RECT *>(lparam) = {0, static_cast<LONG>((static_cast<int>(wparam) - control.TopIndex) * control.ItemHeight),
					control.Rectangle.right - control.Rectangle.left, static_cast<LONG>((static_cast<int>(wparam) - control.TopIndex + 1) * control.ItemHeight)};
				return(0);
			case SBM_GETPOS:
			case TBM_GETPOS: return(control.Position);
			case SBM_SETPOS:
			case TBM_SETPOS:
				{ int const previous = control.Position; control.Position = static_cast<int>(wparam); return(previous); }
			case SBM_SETRANGE:
				control.RangeMinimum = static_cast<int>(wparam); control.RangeMaximum = static_cast<int>(lparam); return(0);
			case TBM_SETRANGE:
				control.RangeMinimum = static_cast<SHORT>(LOWORD(lparam)); control.RangeMaximum = static_cast<SHORT>(HIWORD(lparam)); return(0);
			case SBM_SETSCROLLINFO: {
				auto const * info = reinterpret_cast<SCROLLINFO const *>(lparam);
				if (info) { control.RangeMinimum = info->nMin; control.RangeMaximum = info->nMax; control.Position = info->nPos; }
				return(control.Position);
			}
			case WM_NCHITTEST: return(HTCLIENT);
			default: return(0);
		}
	}

	std::uint64_t Timer_Key(HWND window, UINT_PTR identifier)
	{
		return((static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(window)) << 17) ^ identifier);
	}
}

bool OpenTSMacOS_Is_Control_Window(HWND window)
{
	std::lock_guard lock(ControlMutex);
	return(Find_Control(window) != nullptr);
}

LRESULT OpenTSMacOS_Send_Control_Message(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(0);
	if (control->DialogProcedure) return(control->DialogProcedure(window, message, wparam, lparam));
	if (control->Procedure && control->Procedure != DefWindowProc) return(control->Procedure(window, message, wparam, lparam));
	return(Base_Control_Message(*control, message, wparam, lparam));
}

LRESULT OpenTSMacOS_Def_Control_Message(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	return(control ? Base_Control_Message(*control, message, wparam, lparam) : 0);
}

LRESULT CallWindowProc(WNDPROC procedure, HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	return(procedure ? procedure(window, message, wparam, lparam) : 0);
}

LONG_PTR SetWindowLongPtr(HWND window, int index, LONG_PTR value)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(0);
	LONG_PTR previous = 0;
	switch (index) {
		case GWLP_WNDPROC: previous = reinterpret_cast<LONG_PTR>(control->Procedure); control->Procedure = reinterpret_cast<WNDPROC>(value); break;
		case GWLP_HWNDPARENT: previous = reinterpret_cast<LONG_PTR>(control->Parent); control->Parent = reinterpret_cast<HWND>(value); break;
		case GWL_ID: previous = control->Identifier; control->Identifier = value; break;
		case GWL_STYLE: previous = control->Style; control->Style = value; break;
		case GWL_EXSTYLE: previous = control->ExtendedStyle; control->ExtendedStyle = value; break;
		case DWLP_DLGPROC: previous = reinterpret_cast<LONG_PTR>(control->DialogProcedure); control->DialogProcedure = reinterpret_cast<DLGPROC>(value); break;
		case DWLP_USER: previous = control->UserData; control->UserData = value; break;
		default: previous = control->ExtraValues[index]; control->ExtraValues[index] = value; break;
	}
	return(previous);
}

LONG_PTR GetWindowLongPtr(HWND window, int index)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(0);
	switch (index) {
		case GWLP_WNDPROC: return(reinterpret_cast<LONG_PTR>(control->Procedure));
		case GWLP_HWNDPARENT: return(reinterpret_cast<LONG_PTR>(control->Parent));
		case GWL_ID: return(control->Identifier);
		case GWL_STYLE: return(control->Style);
		case GWL_EXSTYLE: return(control->ExtendedStyle);
		case DWLP_DLGPROC: return(reinterpret_cast<LONG_PTR>(control->DialogProcedure));
		case DWLP_USER: return(control->UserData);
		default: { auto const found = control->ExtraValues.find(index); return(found == control->ExtraValues.end() ? 0 : found->second); }
	}
}

LONG SetWindowLong(HWND window, int index, LONG value) { return(static_cast<LONG>(SetWindowLongPtr(window, index, value))); }
LONG GetWindowLong(HWND window, int index) { return(static_cast<LONG>(GetWindowLongPtr(window, index))); }

ATOM RegisterClass(WNDCLASS const * window_class)
{
	if (!window_class || !window_class->lpszClassName) return(0);
	std::lock_guard lock(ControlMutex);
	Classes[window_class->lpszClassName] = window_class->lpfnWndProc;
	return(1);
}

BOOL UnregisterClass(LPCSTR class_name, HINSTANCE)
{
	std::lock_guard lock(ControlMutex);
	return(class_name && Classes.erase(class_name) != 0);
}

HWND CreateWindowEx(DWORD extended_style, LPCSTR class_name, LPCSTR title, DWORD style,
	int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE, LPVOID parameter)
{
	std::lock_guard lock(ControlMutex);
	auto control = std::make_unique<ControlWindow>();
	control->ClassName = class_name ? class_name : "STATIC";
	control->Text = title ? title : "";
	control->Rectangle = {x, y, x + width, y + height};
	control->Style = style;
	control->ExtendedStyle = extended_style;
	control->Identifier = reinterpret_cast<INT_PTR>(menu);
	control->Parent = parent;
	control->Visible = (style & WS_VISIBLE) != 0;
	control->Enabled = (style & WS_DISABLED) == 0;
	auto const found = Classes.find(control->ClassName);
	if (found != Classes.end()) control->Procedure = found->second;
	HWND const handle = Add_Control(std::move(control));
	if (ControlWindow * added = Find_Control(handle); added && added->Procedure && added->Procedure != DefWindowProc) {
		added->Procedure(handle, WM_CREATE, 0, reinterpret_cast<LPARAM>(parameter));
	}
	return(handle);
}

HWND CreateDialogIndirectParam(HINSTANCE, LPCDLGTEMPLATE dialog_template, HWND parent, DLGPROC procedure, LPARAM parameter)
{
	std::lock_guard lock(ControlMutex);
	auto dialog = std::make_unique<ControlWindow>();
	dialog->ClassName = "#32770";
	dialog->Parent = parent;
	dialog->Rectangle = {0, 0, dialog_template ? dialog_template->cx : 640, dialog_template ? dialog_template->cy : 480};
	dialog->Style = dialog_template ? dialog_template->style : WS_VISIBLE;
	dialog->DialogProcedure = procedure;
	HWND const handle = Add_Control(std::move(dialog));
	if (procedure) procedure(handle, WM_INITDIALOG, 0, parameter);
	return(handle);
}

HWND CreateDialogParam(HINSTANCE instance, LPCSTR, HWND parent, DLGPROC procedure, LPARAM parameter)
{
	return(CreateDialogIndirectParam(instance, nullptr, parent, procedure, parameter));
}

INT_PTR DialogBoxParam(HINSTANCE instance, LPCSTR template_name, HWND parent, DLGPROC procedure, LPARAM parameter)
{
	HWND const dialog = CreateDialogParam(instance, template_name, parent, procedure, parameter);
	if (!dialog) return(-1);
	INT_PTR const result = GetWindowLongPtr(dialog, DWLP_MSGRESULT);
	if (OpenTSMacOS_Is_Control_Window(dialog)) DestroyWindow(dialog);
	return(result != 0 ? result : IDCANCEL);
}

BOOL EndDialog(HWND dialog, INT_PTR result)
{
	SetWindowLongPtr(dialog, DWLP_MSGRESULT, result);
	return(DestroyWindow(dialog));
}

BOOL DestroyWindow(HWND window)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(FALSE);
	auto const children = control->Children;
	for (HWND child : children) DestroyWindow(child);
	if (control->DialogProcedure) control->DialogProcedure(window, WM_DESTROY, 0, 0);
	else if (control->Procedure && control->Procedure != DefWindowProc) control->Procedure(window, WM_DESTROY, 0, 0);
	if (ControlWindow * parent = Find_Control(control->Parent)) {
		parent->Children.erase(std::remove(parent->Children.begin(), parent->Children.end(), window), parent->Children.end());
	}
	if (FocusWindow == window) FocusWindow = nullptr;
	if (CaptureWindow == window) CaptureWindow = nullptr;
	Controls.erase(window);
	return(TRUE);
}

HWND GetDlgItem(HWND dialog, int identifier)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * parent = Find_Control(dialog);
	if (!parent) return(nullptr);
	for (HWND child : parent->Children) {
		ControlWindow * control = Find_Control(child);
		if (control && control->Identifier == identifier) return(child);
	}
	auto control = std::make_unique<ControlWindow>();
	control->Parent = dialog;
	control->Identifier = identifier;
	return(Add_Control(std::move(control)));
}

HWND GetNextDlgTabItem(HWND dialog, HWND control, BOOL previous)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * parent = Find_Control(dialog);
	if (!parent || parent->Children.empty()) return(nullptr);
	auto const & children = parent->Children;
	auto const found = std::find(children.begin(), children.end(), control);
	if (found == children.end()) return(children.front());
	auto const index = static_cast<std::size_t>(found - children.begin());
	auto const count = children.size();
	return(children[previous ? (index + count - 1) % count : (index + 1) % count]);
}

BOOL EnumChildWindows(HWND parent, WNDENUMPROC procedure, LPARAM parameter)
{
	if (!procedure) return(FALSE);
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(parent);
	if (!control) return(FALSE);
	auto const children = control->Children;
	for (HWND child : children) {
		if (!procedure(child, parameter)) return(FALSE);
		EnumChildWindows(child, procedure, parameter);
	}
	return(TRUE);
}

BOOL SetDlgItemText(HWND dialog, int identifier, LPCSTR text) { return(SetWindowText(GetDlgItem(dialog, identifier), text)); }
UINT GetDlgItemText(HWND dialog, int identifier, LPSTR text, int size) { return(GetWindowText(GetDlgItem(dialog, identifier), text, size)); }
LRESULT SendDlgItemMessage(HWND dialog, int identifier, UINT message, WPARAM wparam, LPARAM lparam) { return(SendMessage(GetDlgItem(dialog, identifier), message, wparam, lparam)); }
BOOL CheckDlgButton(HWND dialog, int identifier, UINT check) { SendMessage(GetDlgItem(dialog, identifier), BM_SETCHECK, check, 0); return(TRUE); }
UINT IsDlgButtonChecked(HWND dialog, int identifier) { return(static_cast<UINT>(SendMessage(GetDlgItem(dialog, identifier), BM_GETCHECK, 0, 0))); }

BOOL EnableWindow(HWND window, BOOL enable)
{
	std::lock_guard lock(ControlMutex);
	if (ControlWindow * control = Find_Control(window)) { control->Enabled = enable != 0; return(TRUE); }
	return(FALSE);
}

BOOL IsWindowEnabled(HWND window)
{
	std::lock_guard lock(ControlMutex);
	if (ControlWindow * control = Find_Control(window)) return(control->Enabled);
	return(FALSE);
}

BOOL IsWindowVisible(HWND window)
{
	std::lock_guard lock(ControlMutex);
	if (ControlWindow * control = Find_Control(window)) return(control->Visible);
	return(window != nullptr);
}

BOOL IsWindow(HWND window)
{
	return(window != nullptr && (OpenTSMacOS_Is_Control_Window(window) || window != nullptr));
}

BOOL IsChild(HWND parent, HWND window)
{
	std::lock_guard lock(ControlMutex);
	for (ControlWindow * control = Find_Control(window); control; control = Find_Control(control->Parent)) {
		if (control->Parent == parent) return(TRUE);
	}
	return(FALSE);
}

BOOL ShowWindow(HWND window, int command)
{
	std::lock_guard lock(ControlMutex);
	if (ControlWindow * control = Find_Control(window)) { control->Visible = command != SW_HIDE; return(TRUE); }
	return(window != nullptr);
}

BOOL UpdateWindow(HWND window) { return(InvalidateRect(window, nullptr, FALSE)); }
BOOL CloseWindow(HWND window) { return(ShowWindow(window, SW_HIDE)); }
HWND SetCapture(HWND window) { HWND const previous = CaptureWindow; CaptureWindow = window; return(previous); }
HWND GetCapture(void) { return(CaptureWindow); }
BOOL ReleaseCapture(void) { CaptureWindow = nullptr; return(TRUE); }
HWND GetFocus(void) { return(FocusWindow); }

HWND OpenTSMacOS_Set_Control_Focus(HWND window)
{
	std::lock_guard lock(ControlMutex);
	HWND const previous = FocusWindow;
	FocusWindow = window;
	return(previous);
}

HWND GetParent(HWND window)
{
	std::lock_guard lock(ControlMutex);
	if (ControlWindow * control = Find_Control(window)) return(control->Parent);
	return(nullptr);
}

HWND GetWindow(HWND window, UINT command)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(nullptr);
	if (command == GW_CHILD) return(control->Children.empty() ? nullptr : control->Children.front());
	ControlWindow * parent = Find_Control(control->Parent);
	if (!parent) return(command == GW_OWNER ? control->Parent : nullptr);
	auto const found = std::find(parent->Children.begin(), parent->Children.end(), window);
	if (found == parent->Children.end()) return(nullptr);
	if (command == GW_HWNDNEXT && found + 1 != parent->Children.end()) return(*(found + 1));
	if (command == GW_HWNDPREV && found != parent->Children.begin()) return(*(found - 1));
	if (command == GW_HWNDFIRST) return(parent->Children.front());
	if (command == GW_HWNDLAST) return(parent->Children.back());
	return(nullptr);
}

HWND GetTopWindow(HWND parent)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(parent);
	return(control && !control->Children.empty() ? control->Children.front() : nullptr);
}

int GetClassName(HWND window, LPSTR class_name, int size)
{
	if (!class_name || size <= 0) return(0);
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) { class_name[0] = '\0'; return(0); }
	int const length = std::min(size - 1, static_cast<int>(control->ClassName.size()));
	std::memcpy(class_name, control->ClassName.data(), length);
	class_name[length] = '\0';
	return(length);
}

BOOL SetWindowText(HWND window, LPCSTR text)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(FALSE);
	control->Text.assign(text ? text : "", 0, control->TextLimit);
	return(TRUE);
}

int OpenTSMacOS_Get_Control_Text(HWND window, LPSTR text, int size)
{
	if (!text || size <= 0) return(0);
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) { text[0] = '\0'; return(0); }
	int const length = std::min(size - 1, static_cast<int>(control->Text.size()));
	std::memcpy(text, control->Text.data(), length);
	text[length] = '\0';
	return(length);
}

int GetWindowTextLength(HWND window)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	return(control ? static_cast<int>(control->Text.size()) : 0);
}

BOOL OpenTSMacOS_Get_Control_Rect(HWND window, RECT * rectangle, BOOL client)
{
	if (!rectangle) return(FALSE);
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(FALSE);
	if (client) *rectangle = {0, 0, control->Rectangle.right - control->Rectangle.left, control->Rectangle.bottom - control->Rectangle.top};
	else *rectangle = control->Rectangle;
	return(TRUE);
}

BOOL OpenTSMacOS_Control_Point_Transform(HWND window, POINT * point, BOOL to_screen)
{
	if (!point) return(FALSE);
	std::lock_guard lock(ControlMutex);
	for (ControlWindow * control = Find_Control(window); control; control = Find_Control(control->Parent)) {
		point->x += to_screen ? control->Rectangle.left : -control->Rectangle.left;
		point->y += to_screen ? control->Rectangle.top : -control->Rectangle.top;
	}
	return(TRUE);
}

BOOL GetWindowRect(HWND window, RECT * rectangle)
{
	return(OpenTSMacOS_Is_Control_Window(window)
		? OpenTSMacOS_Get_Control_Rect(window, rectangle, FALSE)
		: OpenTSMacOS_Get_Native_Window_Rect(window, rectangle));
}

BOOL MoveWindow(HWND window, int x, int y, int width, int height, BOOL repaint)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(window);
	if (!control) return(OpenTSMacOS_Move_Native_Window(window, x, y, width, height, repaint));
	control->Rectangle = {x, y, x + width, y + height};
	if (repaint) InvalidateRect(window, nullptr, FALSE);
	return(TRUE);
}

BOOL SetWindowPos(HWND window, HWND, int x, int y, int width, int height, UINT flags)
{
	RECT rectangle = {};
	if (!GetWindowRect(window, &rectangle)) return(FALSE);
	if (flags & SWP_NOMOVE) { x = rectangle.left; y = rectangle.top; }
	if (flags & SWP_NOSIZE) { width = rectangle.right - rectangle.left; height = rectangle.bottom - rectangle.top; }
	return(MoveWindow(window, x, y, width, height, TRUE));
}

BOOL SetRect(RECT * rectangle, int left, int top, int right, int bottom)
{
	if (!rectangle) return(FALSE);
	*rectangle = {left, top, right, bottom};
	return(TRUE);
}

BOOL PtInRect(RECT const * rectangle, POINT point)
{
	return(rectangle && point.x >= rectangle->left && point.x < rectangle->right
		&& point.y >= rectangle->top && point.y < rectangle->bottom);
}

int MapWindowPoints(HWND source, HWND destination, POINT * points, UINT count)
{
	if (!points) return(0);
	for (UINT index = 0; index < count; ++index) {
		if (source && OpenTSMacOS_Is_Control_Window(source)) OpenTSMacOS_Control_Point_Transform(source, &points[index], TRUE);
		if (destination && OpenTSMacOS_Is_Control_Window(destination)) OpenTSMacOS_Control_Point_Transform(destination, &points[index], FALSE);
	}
	return(0);
}

HWND ChildWindowFromPoint(HWND parent, POINT point)
{
	std::lock_guard lock(ControlMutex);
	ControlWindow * control = Find_Control(parent);
	if (!control) return(nullptr);
	for (auto child = control->Children.rbegin(); child != control->Children.rend(); ++child) {
		ControlWindow * candidate = Find_Control(*child);
		if (candidate && candidate->Visible && point.x >= candidate->Rectangle.left && point.x < candidate->Rectangle.right
		&& point.y >= candidate->Rectangle.top && point.y < candidate->Rectangle.bottom) return(*child);
	}
	return(parent);
}

HMONITOR MonitorFromWindow(HWND, DWORD) { return(reinterpret_cast<HMONITOR>(1)); }
BOOL GetMonitorInfo(HMONITOR, MONITORINFO * information)
{
	if (!information) return(FALSE);
	information->rcMonitor = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
	information->rcWork = information->rcMonitor;
	information->dwFlags = 1;
	return(TRUE);
}

BOOL OpenTSMacOS_Invalidate_Control(HWND window) { return(OpenTSMacOS_Is_Control_Window(window)); }

BOOL GetUpdateRect(HWND window, RECT * rectangle, BOOL)
{
	if (!rectangle) return(window != nullptr);
	return(GetClientRect(window, rectangle));
}

int GetBkMode(HDC) { return(TRANSPARENT); }
COLORREF GetBkColor(HDC) { return(RGB(0, 0, 0)); }
COLORREF GetTextColor(HDC) { return(RGB(255, 255, 255)); }
COLORREF SetBkColor(HDC, COLORREF color) { return(color); }

BOOL IntersectRect(RECT * destination, RECT const * left, RECT const * right)
{
	if (!destination || !left || !right) return(FALSE);
	destination->left = std::max(left->left, right->left);
	destination->top = std::max(left->top, right->top);
	destination->right = std::min(left->right, right->right);
	destination->bottom = std::min(left->bottom, right->bottom);
	if (destination->right <= destination->left || destination->bottom <= destination->top) {
		*destination = {};
		return(FALSE);
	}
	return(TRUE);
}

HWND WindowFromPoint(POINT point)
{
	std::lock_guard lock(ControlMutex);
	for (auto const & entry : Controls) {
		ControlWindow const & control = *entry.second;
		if (control.Visible && PtInRect(&control.Rectangle, point)) return(entry.first);
	}
	return(nullptr);
}

BOOL AdjustWindowRectEx(RECT * rectangle, DWORD, BOOL, DWORD) { return(rectangle != nullptr); }
BOOL RedrawWindow(HWND window, RECT const *, HANDLE, UINT) { return(InvalidateRect(window, nullptr, TRUE)); }
BOOL BringWindowToTop(HWND window) { return(window != nullptr); }
int GetDlgCtrlID(HWND window) { return(static_cast<int>(GetWindowLongPtr(window, GWL_ID))); }

UINT_PTR SetTimer(HWND window, UINT_PTR identifier, UINT interval, TIMERPROC procedure)
{
	if (identifier == 0) identifier = 1;
	auto active = std::make_shared<std::atomic<bool>>(true);
	{
		std::lock_guard lock(ControlMutex);
		Timers[Timer_Key(window, identifier)] = active;
	}
	std::thread([window, identifier, interval = std::max(1u, interval), procedure, active]() {
		while (active->load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			if (!active->load()) break;
			if (procedure) procedure(window, WM_TIMER, identifier, static_cast<DWORD>(GetTickCount()));
			else PostMessage(window, WM_TIMER, identifier, 0);
		}
	}).detach();
	return(identifier);
}

BOOL KillTimer(HWND window, UINT_PTR identifier)
{
	std::lock_guard lock(ControlMutex);
	auto const found = Timers.find(Timer_Key(window, identifier));
	if (found == Timers.end()) return(FALSE);
	found->second->store(false);
	Timers.erase(found);
	return(TRUE);
}
