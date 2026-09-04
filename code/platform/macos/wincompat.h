/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <thread>
#include <strings.h>

#include <arpa/inet.h>
#include <crt_externs.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <iconv.h>
#include <netdb.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __fastcall
#define __fastcall
#endif
#define CALLBACK
#define WINAPI
#define APIENTRY
#define far
#define EXTERN_C extern "C"
#define STDMETHODCALLTYPE
#define STDAPICALLTYPE
#define STDMETHOD(method) virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type, method) virtual type STDMETHODCALLTYPE method
#define STDMETHODIMP HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
#define PURE = 0
#define interface struct
#define DECLSPEC_UUID(value) __declspec(uuid(value))
#define MIDL_INTERFACE(value) struct DECLSPEC_UUID(value)

using BOOL = int;
using boolean = unsigned char;
using VOID = void;
using BYTE = std::uint8_t;
using CHAR = char;
using WCHAR = wchar_t;
using UCHAR = unsigned char;
using SHORT = std::int16_t;
using USHORT = std::uint16_t;
using WORD = std::uint16_t;
using INT = int;
using UINT = unsigned int;
using LONG = std::int32_t;
using ULONG = std::uint32_t;
using DWORD = std::uint32_t;
using LONGLONG = std::int64_t;
using ULONGLONG = std::uint64_t;
using INT_PTR = std::intptr_t;
using UINT_PTR = std::uintptr_t;
using LONG_PTR = std::intptr_t;
using ULONG_PTR = std::uintptr_t;
using DWORD_PTR = std::uintptr_t;
using WPARAM = std::uintptr_t;
using LPARAM = std::intptr_t;
using LRESULT = std::intptr_t;
using HRESULT = std::int32_t;
using MMRESULT = unsigned int;
using ATOM = unsigned short;
using SOCKET = int;
using LINGER = struct linger;
using LPSOCKADDR = struct sockaddr *;
using LANGID = unsigned short;
using LCID = std::uint32_t;
using DISPID = long;
using VARTYPE = unsigned short;
using VARIANT_BOOL = short;
constexpr VARIANT_BOOL VARIANT_FALSE = 0;
constexpr VARIANT_BOOL VARIANT_TRUE = -1;

#ifndef TEXT
#define TEXT(value) value
#endif

using HANDLE = void *;
using HINSTANCE = void *;
using HMODULE = void *;
using HWND = void *;
using HDC = void *;
using HBITMAP = void *;
using HGDIOBJ = void *;
using HBRUSH = void *;
using HPEN = void *;
using HFONT = void *;
using HICON = void *;
using HCURSOR = void *;
using HMENU = void *;
using HGLOBAL = void *;
using HRSRC = void *;
using HACCEL = void *;
using HMONITOR = void *;
using HIMAGELIST = void *;
using HTREEITEM = void *;
using HKEY = void *;
using COLORREF = DWORD;
using FARPROC = void (*)();
using LPVOID = void *;
using LPCVOID = void const *;
using LPSTR = char *;
using LPCSTR = char const *;
using LPCTSTR = char const *;
using LPWSTR = wchar_t *;
using LPCWSTR = wchar_t const *;
using LPOLESTR = wchar_t *;
using LPBYTE = BYTE *;
using PBYTE = BYTE *;
using LPWORD = WORD *;
using LPDWORD = DWORD *;

#ifndef FALSE
constexpr BOOL FALSE = 0;
#endif
#ifndef TRUE
constexpr BOOL TRUE = 1;
#endif
constexpr HRESULT S_OK = 0;
constexpr HRESULT S_FALSE = 1;
constexpr HRESULT E_FAIL = static_cast<HRESULT>(0x80004005L);
constexpr HRESULT E_NOINTERFACE = static_cast<HRESULT>(0x80004002L);
constexpr HRESULT E_NOTIMPL = static_cast<HRESULT>(0x80004001L);
constexpr HRESULT E_POINTER = static_cast<HRESULT>(0x80004003L);
constexpr HRESULT E_INVALIDARG = static_cast<HRESULT>(0x80070057L);
constexpr HRESULT E_OUTOFMEMORY = static_cast<HRESULT>(0x8007000EL);
constexpr HRESULT CLASS_E_NOAGGREGATION = static_cast<HRESULT>(0x80040110L);
constexpr HRESULT STG_E_INVALIDFUNCTION = static_cast<HRESULT>(0x80030001L);
constexpr HRESULT STG_E_FILENOTFOUND = static_cast<HRESULT>(0x80030002L);
constexpr HRESULT STG_E_ACCESSDENIED = static_cast<HRESULT>(0x80030005L);
constexpr HRESULT STG_E_READFAULT = static_cast<HRESULT>(0x8003001EL);
constexpr HRESULT STG_E_WRITEFAULT = static_cast<HRESULT>(0x8003001DL);
constexpr HRESULT STG_E_INVALIDPOINTER = static_cast<HRESULT>(0x80030009L);
constexpr HRESULT STG_E_FILEALREADYEXISTS = static_cast<HRESULT>(0x80030050L);
constexpr HRESULT STG_E_MEDIUMFULL = static_cast<HRESULT>(0x80030070L);
constexpr UINT CP_ACP = 0;
constexpr DWORD MB_PRECOMPOSED = 1;
constexpr WORD LANG_NEUTRAL = 0;
constexpr WORD SUBLANG_DEFAULT = 1;
#define MAKELANGID(primary, sublanguage) static_cast<LANGID>((static_cast<WORD>(sublanguage) << 10) | static_cast<WORD>(primary))

#define SUCCEEDED(result) ((HRESULT)(result) >= 0)
#define FAILED(result) ((HRESULT)(result) < 0)
#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(-1))
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define MAX_PATH 1024
#define _MAX_PATH MAX_PATH
#define _MAX_FNAME 256
#define _MAX_EXT 256
#define _MAX_DRIVE 4
#define _MAX_DIR MAX_PATH

struct POINT { LONG x; LONG y; };
struct POINTS { SHORT x; SHORT y; };
using LPPOINT = POINT *;
struct SIZE { LONG cx; LONG cy; };
struct RECT { LONG left; LONG top; LONG right; LONG bottom; };
using tagRECT = RECT;
struct FILETIME { DWORD dwLowDateTime; DWORD dwHighDateTime; };
struct SYSTEMTIME {
	WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds;
};
#ifndef OPENTS_TIMEB_DEFINED
#define OPENTS_TIMEB_DEFINED
struct timeb { time_t time; unsigned short millitm; short timezone; short dstflag; };
inline int ftime(timeb * value)
{
	if (!value) return(-1);
	timeval current = {};
	if (gettimeofday(&current, nullptr) != 0) return(-1);
	value->time = current.tv_sec;
	value->millitm = static_cast<unsigned short>(current.tv_usec / 1000);
	value->timezone = 0;
	value->dstflag = 0;
	return(0);
}
inline int _ftime(timeb * value) { return(ftime(value)); }
#define _timeb timeb
#endif
struct MEMORYSTATUS
{
	DWORD dwLength;
	DWORD dwMemoryLoad;
	size_t dwTotalPhys;
	size_t dwAvailPhys;
	size_t dwTotalPageFile;
	size_t dwAvailPageFile;
	size_t dwTotalVirtual;
	size_t dwAvailVirtual;
};

inline void GlobalMemoryStatus(MEMORYSTATUS * status)
{
	if (!status) return;
	std::uint64_t total = 0;
	size_t length = sizeof(total);
	if (sysctlbyname("hw.memsize", &total, &length, nullptr, 0) != 0) total = 0;
	status->dwTotalPhys = static_cast<size_t>(total);
	status->dwAvailPhys = 0;
	status->dwTotalPageFile = status->dwAvailPageFile = 0;
	status->dwTotalVirtual = status->dwAvailVirtual = 0;
	status->dwMemoryLoad = 0;
}
union LARGE_INTEGER {
	struct { DWORD LowPart; LONG HighPart; };
	LONGLONG QuadPart;
};
union ULARGE_INTEGER {
	struct { DWORD LowPart; DWORD HighPart; };
	ULONGLONG QuadPart;
};
struct MSG { HWND hwnd; UINT message; WPARAM wParam; LPARAM lParam; DWORD time; POINT pt; };
struct DRAWITEMSTRUCT {
	UINT CtlType;
	UINT CtlID;
	UINT itemID;
	UINT itemAction;
	UINT itemState;
	HWND hwndItem;
	HDC hDC;
	RECT rcItem;
	ULONG_PTR itemData;
};
using LPDRAWITEMSTRUCT = DRAWITEMSTRUCT *;
constexpr UINT ODT_BUTTON = 4;
using LPRECT = RECT *;
using PULARGE_INTEGER = ULARGE_INTEGER *;
using DLGPROC = INT_PTR (CALLBACK *)(HWND, UINT, WPARAM, LPARAM);
using WNDPROC = LRESULT (CALLBACK *)(HWND, UINT, WPARAM, LPARAM);
using WNDENUMPROC = BOOL (CALLBACK *)(HWND, LPARAM);
using TIMERPROC = void (CALLBACK *)(HWND, UINT, UINT_PTR, DWORD);

struct DLGTEMPLATE { DWORD style; DWORD dwExtendedStyle; WORD cdit; SHORT x, y, cx, cy; };
using LPCDLGTEMPLATE = DLGTEMPLATE const *;
struct WNDCLASS {
	UINT style;
	WNDPROC lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCSTR lpszMenuName;
	LPCSTR lpszClassName;
};
struct MONITORINFO { DWORD cbSize; RECT rcMonitor; RECT rcWork; DWORD dwFlags; };
struct PAINTSTRUCT { HDC hdc; BOOL fErase; RECT rcPaint; BOOL fRestore; BOOL fIncUpdate; BYTE rgbReserved[32]; };
struct TEXTMETRIC {
	LONG tmHeight, tmAscent, tmDescent, tmInternalLeading, tmExternalLeading;
	LONG tmAveCharWidth, tmMaxCharWidth, tmWeight, tmOverhang;
	LONG tmDigitizedAspectX, tmDigitizedAspectY;
	CHAR tmFirstChar, tmLastChar, tmDefaultChar, tmBreakChar;
	BYTE tmItalic, tmUnderlined, tmStruckOut, tmPitchAndFamily, tmCharSet;
};
struct LOGFONTA {
	LONG lfHeight, lfWidth, lfEscapement, lfOrientation, lfWeight;
	BYTE lfItalic, lfUnderline, lfStrikeOut, lfCharSet, lfOutPrecision, lfClipPrecision, lfQuality, lfPitchAndFamily;
	CHAR lfFaceName[32];
};
using LOGFONT = LOGFONTA;
struct HELPINFO { UINT cbSize; int iContextType; int iCtrlId; HANDLE hItemHandle; DWORD_PTR dwContextId; POINT MousePos; };
struct WINDOWPOS { HWND hwnd; HWND hwndInsertAfter; int x, y, cx, cy; UINT flags; };
struct OSVERSIONINFOA {
	DWORD dwOSVersionInfoSize, dwMajorVersion, dwMinorVersion, dwBuildNumber, dwPlatformId;
	CHAR szCSDVersion[128];
};
using OSVERSIONINFO = OSVERSIONINFOA;
struct SCROLLINFO { UINT cbSize; UINT fMask; int nMin; int nMax; UINT nPage; int nPos; int nTrackPos; };
using LPSCROLLINFO = SCROLLINFO *;

constexpr int DWLP_USER = 0;
constexpr int DWLP_MSGRESULT = 1;
constexpr int DWLP_DLGPROC = 2;
constexpr int GWL_WNDPROC = -4;
constexpr int GWL_HINSTANCE = -6;
constexpr int GWL_HWNDPARENT = -8;
constexpr int GWL_ID = -12;
constexpr int GWL_STYLE = -16;
constexpr int GWL_EXSTYLE = -20;
constexpr int GWLP_WNDPROC = GWL_WNDPROC;
constexpr int GWLP_HWNDPARENT = GWL_HWNDPARENT;
constexpr int GWLP_ID = GWL_ID;
constexpr int GW_HWNDFIRST = 0;
constexpr int GW_HWNDLAST = 1;
constexpr int GW_HWNDNEXT = 2;
constexpr int GW_HWNDPREV = 3;
constexpr int GW_OWNER = 4;
constexpr int GW_CHILD = 5;
constexpr int SW_HIDE = 0;
constexpr int SW_NORMAL = 1;
constexpr int SW_RESTORE = 9;
constexpr int SW_SHOWNORMAL = 1;
constexpr int SW_SHOW = 5;
constexpr int IDCANCEL = 2;
constexpr int IDOK = 1;
constexpr UINT WM_CREATE = 0x0001;
constexpr UINT WM_DESTROY = 0x0002;
constexpr UINT WM_MOVE = 0x0003;
constexpr UINT WM_SIZE = 0x0005;
constexpr UINT WM_ACTIVATE = 0x0006;
constexpr UINT WM_SETFOCUS = 0x0007;
constexpr UINT WM_KILLFOCUS = 0x0008;
constexpr UINT WM_ENABLE = 0x000A;
constexpr UINT WM_PAINT = 0x000F;
constexpr UINT WM_CLOSE = 0x0010;
constexpr UINT WM_QUIT = 0x0012;
constexpr UINT WM_ERASEBKGND = 0x0014;

constexpr UINT WM_SETTEXT = 0x000C;
constexpr UINT WM_GETTEXT = 0x000D;
constexpr UINT WM_GETTEXTLENGTH = 0x000E;
constexpr UINT WM_SHOWWINDOW = 0x0018;
constexpr UINT WM_ACTIVATEAPP = 0x001C;
constexpr UINT WM_SETCURSOR = 0x0020;
constexpr UINT WM_MOUSEACTIVATE = 0x0021;
constexpr UINT WM_SETFONT = 0x0030;
constexpr UINT WM_DISPLAYCHANGE = 0x007E;
constexpr UINT WM_KEYDOWN = 0x0100;
constexpr UINT WM_KEYUP = 0x0101;
constexpr UINT WM_CHAR = 0x0102;
constexpr UINT WM_SYSKEYDOWN = 0x0104;
constexpr UINT WM_SYSKEYUP = 0x0105;
constexpr UINT WM_SYSCHAR = 0x0106;
constexpr UINT WM_SYSDEADCHAR = 0x0107;
constexpr UINT WM_COMMAND = 0x0111;
constexpr UINT WM_SYSCOLORCHANGE = 0x0015;
constexpr UINT WM_HELP = 0x0053;
constexpr UINT WM_CONTEXTMENU = 0x007B;
constexpr UINT WM_NOTIFY = 0x004E;
constexpr UINT WM_WINDOWPOSCHANGING = 0x0046;
constexpr UINT WM_WINDOWPOSCHANGED = 0x0047;
constexpr UINT WM_NCDESTROY = 0x0082;
constexpr UINT WM_NCHITTEST = 0x0084;
constexpr UINT WM_NCPAINT = 0x0085;
constexpr UINT WM_GETDLGCODE = 0x0087;
constexpr UINT WM_NCMOUSEMOVE = 0x00A0;
constexpr UINT WM_INITDIALOG = 0x0110;
constexpr UINT WM_HSCROLL = 0x0114;
constexpr UINT WM_VSCROLL = 0x0115;
constexpr UINT WM_TIMER = 0x0113;
constexpr UINT WM_DRAWITEM = 0x002B;
constexpr UINT WM_CTLCOLORMSGBOX = 0x0132;
constexpr UINT WM_CTLCOLOREDIT = 0x0133;
constexpr UINT WM_CTLCOLORLISTBOX = 0x0134;
constexpr UINT WM_CTLCOLORBTN = 0x0135;
constexpr UINT WM_CTLCOLORDLG = 0x0136;
constexpr UINT WM_CTLCOLORSCROLLBAR = 0x0137;
constexpr UINT WM_CTLCOLORSTATIC = 0x0138;
constexpr UINT WM_USER = 0x0400;
constexpr UINT WM_MOUSEMOVE = 0x0200;
constexpr UINT WM_LBUTTONDOWN = 0x0201;
constexpr UINT WM_LBUTTONUP = 0x0202;
constexpr UINT WM_LBUTTONDBLCLK = 0x0203;
constexpr UINT WM_RBUTTONDOWN = 0x0204;
constexpr UINT WM_RBUTTONUP = 0x0205;
constexpr UINT WM_RBUTTONDBLCLK = 0x0206;
constexpr UINT WM_MBUTTONDOWN = 0x0207;
constexpr UINT WM_MBUTTONUP = 0x0208;
constexpr UINT WM_MBUTTONDBLCLK = 0x0209;
constexpr UINT WM_MOUSEWHEEL = 0x020A;
constexpr UINT WM_XBUTTONDOWN = 0x020B;
constexpr UINT WM_XBUTTONUP = 0x020C;
constexpr UINT WM_XBUTTONDBLCLK = 0x020D;
constexpr UINT WM_MOUSELAST = WM_MOUSEWHEEL;
constexpr UINT WM_KEYLAST = 0x0109;
constexpr UINT WM_CAPTURECHANGED = 0x0215;
constexpr UINT WM_MOVING = 0x0216;
constexpr UINT WM_SYSCOMMAND = 0x0112;
constexpr UINT WM_APP = 0x8000;
constexpr WPARAM SIZE_MINIMIZED = 1;
constexpr UINT PM_NOREMOVE = 0;
constexpr UINT PM_REMOVE = 1;
constexpr int HTCLIENT = 1;
constexpr int HTTRANSPARENT = -1;
constexpr WPARAM SC_CLOSE = 0xF060;
constexpr WPARAM SC_SCREENSAVE = 0xF140;
constexpr UINT BN_CLICKED = 0;
constexpr UINT CBN_SELCHANGE = 1;
constexpr UINT LBN_SELCHANGE = 1;
constexpr UINT EN_CHANGE = 0x0300;
constexpr UINT EN_MAXTEXT = 0x0501;
constexpr UINT LBN_DBLCLK = 2;
constexpr UINT BST_UNCHECKED = 0;
constexpr UINT BST_CHECKED = 1;
constexpr UINT BST_INDETERMINATE = 2;
constexpr LRESULT CB_ERR = -1;
constexpr LRESULT LB_ERR = -1;
constexpr UINT BM_GETCHECK = 0x00F0;
constexpr UINT BM_SETCHECK = 0x00F1;
constexpr UINT BM_GETSTATE = 0x00F2;
constexpr UINT BM_SETSTATE = 0x00F3;
constexpr UINT BM_CLICK = 0x00F5;
constexpr UINT EM_GETSEL = 0x00B0;
constexpr UINT EM_SETSEL = 0x00B1;
constexpr UINT EM_POSFROMCHAR = 0x00D6;
constexpr UINT EM_SETLIMITTEXT = 0x00C5;
constexpr UINT EM_LIMITTEXT = EM_SETLIMITTEXT;
constexpr UINT CB_GETEDITSEL = 0x0140;
constexpr UINT CB_LIMITTEXT = 0x0141;
constexpr UINT CB_SETEDITSEL = 0x0142;
constexpr UINT CB_ADDSTRING = 0x0143;
constexpr UINT CB_DELETESTRING = 0x0144;
constexpr UINT CB_GETCOUNT = 0x0146;
constexpr UINT CB_GETCURSEL = 0x0147;
constexpr UINT CB_GETLBTEXT = 0x0148;
constexpr UINT CB_GETLBTEXTLEN = 0x0149;
constexpr UINT CB_INSERTSTRING = 0x014A;
constexpr UINT CB_RESETCONTENT = 0x014B;
constexpr UINT CB_FINDSTRING = 0x014C;
constexpr UINT CB_SELECTSTRING = 0x014D;
constexpr UINT CB_SETCURSEL = 0x014E;
constexpr UINT CB_SHOWDROPDOWN = 0x014F;
constexpr UINT CB_GETITEMDATA = 0x0150;
constexpr UINT CB_SETITEMDATA = 0x0151;
constexpr UINT CB_GETDROPPEDCONTROLRECT = 0x0152;
constexpr UINT CB_SETITEMHEIGHT = 0x0153;
constexpr UINT CB_GETITEMHEIGHT = 0x0154;
constexpr UINT CB_GETDROPPEDSTATE = 0x0157;
constexpr UINT CB_FINDSTRINGEXACT = 0x0158;
constexpr UINT CB_SETTOPINDEX = 0x015C;
constexpr UINT CB_GETTOPINDEX = 0x015B;
constexpr UINT LB_ADDSTRING = 0x0180;
constexpr UINT LB_INSERTSTRING = 0x0181;
constexpr UINT LB_DELETESTRING = 0x0182;
constexpr UINT LB_RESETCONTENT = 0x0184;
constexpr UINT LB_SETSEL = 0x0185;
constexpr UINT LB_SETCURSEL = 0x0186;
constexpr UINT LB_GETSEL = 0x0187;
constexpr UINT LB_GETCURSEL = 0x0188;
constexpr UINT LB_GETTEXT = 0x0189;
constexpr UINT LB_GETTEXTLEN = 0x018A;
constexpr UINT LB_GETCOUNT = 0x018B;
constexpr UINT LB_SELECTSTRING = 0x018C;
constexpr UINT LB_GETTOPINDEX = 0x018E;
constexpr UINT LB_FINDSTRING = 0x018F;
constexpr UINT LB_GETSELCOUNT = 0x0190;
constexpr UINT LB_GETSELITEMS = 0x0191;
constexpr UINT LB_SETTABSTOPS = 0x0192;
constexpr UINT LB_GETHORIZONTALEXTENT = 0x0193;
constexpr UINT LB_SETHORIZONTALEXTENT = 0x0194;
constexpr UINT LB_SETCOLUMNWIDTH = 0x0195;
constexpr UINT LB_ADDFILE = 0x0196;
constexpr UINT LB_SETTOPINDEX = 0x0197;
constexpr UINT LB_GETITEMRECT = 0x0198;
constexpr UINT LB_GETITEMDATA = 0x0199;
constexpr UINT LB_SETITEMDATA = 0x019A;
constexpr UINT LB_SELITEMRANGE = 0x019B;
constexpr UINT LB_SETITEMHEIGHT = 0x01A0;
constexpr UINT LB_GETITEMHEIGHT = 0x01A1;
constexpr UINT LB_FINDSTRINGEXACT = 0x01A2;
constexpr UINT SBM_SETPOS = 0x00E0;
constexpr UINT SBM_GETPOS = 0x00E1;
constexpr UINT SBM_SETRANGE = 0x00E2;
constexpr UINT SBM_GETRANGE = 0x00E3;
constexpr UINT SBM_SETSCROLLINFO = 0x00E9;
constexpr UINT SBM_GETSCROLLINFO = 0x00EA;
constexpr UINT SB_LINEUP = 0;
constexpr UINT SB_LINEDOWN = 1;
constexpr UINT SB_THUMBTRACK = 5;
constexpr UINT SB_THUMBPOSITION = 4;
constexpr UINT SB_ENDSCROLL = 8;
constexpr UINT SWP_NOSIZE = 0x0001;
constexpr UINT SWP_NOMOVE = 0x0002;
constexpr UINT SWP_NOZORDER = 0x0004;
constexpr UINT SWP_NOACTIVATE = 0x0010;
constexpr UINT SWP_NOOWNERZORDER = 0x0200;
constexpr DWORD MONITOR_DEFAULTTONEAREST = 2;
constexpr UINT CS_VREDRAW = 0x0001;
constexpr UINT CS_HREDRAW = 0x0002;
constexpr LONG WS_OVERLAPPED = 0;
constexpr LONG WS_VISIBLE = 0x10000000L;
constexpr LONG WS_DISABLED = 0x08000000L;
constexpr LONG WS_CHILD = 0x40000000L;
constexpr LONG WS_BORDER = 0x00800000L;
constexpr LONG WS_TABSTOP = 0x00010000L;
constexpr LONG ES_MULTILINE = 0x0004L;
constexpr LONG ES_PASSWORD = 0x0020L;
constexpr LONG SS_CENTER = 0x0001L;
constexpr LONG SS_RIGHT = 0x0002L;
constexpr UINT MK_LBUTTON = 0x0001;
constexpr LONG LBS_MULTIPLESEL = 0x0008L;
constexpr LONG LBS_EXTENDEDSEL = 0x0800L;
constexpr LONG LBS_NOSEL = 0x4000L;
constexpr LONG BS_AUTOCHECKBOX = 0x0003L;
constexpr LONG BS_GROUPBOX = 0x0007L;
constexpr LONG BS_OWNERDRAW = 0x000BL;
constexpr LONG DS_SETFONT = 0x0040L;
constexpr UINT SIF_RANGE = 0x0001;
constexpr UINT SIF_PAGE = 0x0002;
constexpr UINT SIF_POS = 0x0004;
constexpr UINT RDW_INVALIDATE = 0x0001;
constexpr UINT RDW_INTERNALPAINT = 0x0002;
constexpr UINT RDW_ERASE = 0x0004;
constexpr UINT RDW_ALLCHILDREN = 0x0080;
constexpr UINT RDW_UPDATENOW = 0x0100;
constexpr UINT RDW_FRAME = 0x0400;
constexpr int BLACK_BRUSH = 4;
constexpr int NULL_BRUSH = 5;
constexpr int SYSTEM_FONT = 13;
constexpr int TRANSPARENT = 1;
constexpr UINT DT_SINGLELINE = 0x0020;
constexpr UINT DT_VCENTER = 0x0004;
constexpr int GM_ADVANCED = 2;
constexpr DWORD MWT_IDENTITY = 1;
constexpr BYTE ANSI_CHARSET = 0;
constexpr LANGID LANG_USER_DEFAULT = 0x0400;
constexpr DWORD TIME_NOMINUTESORSECONDS = 0x00000001;
constexpr DWORD TIME_NOSECONDS = 0x00000002;
constexpr DWORD KEY_READ = 0x20019;
constexpr LONG ERROR_SUCCESS = 0;
constexpr UINT HELP_CONTEXTPOPUP = 0x0008;
constexpr UINT HELP_CONTEXTMENU = 0x000A;
constexpr DWORD VER_PLATFORM_WIN32_WINDOWS = 1;
constexpr DWORD VER_PLATFORM_WIN32_NT = 2;
#define HKEY_LOCAL_MACHINE reinterpret_cast<HKEY>(static_cast<UINT_PTR>(0x80000002u))
#define HWND_DESKTOP nullptr
#define IDC_ARROW MAKEINTRESOURCE(32512)
#define IDC_NO MAKEINTRESOURCE(32648)
constexpr DWORD RT_DIALOG_VALUE = 5;
#define RT_DIALOG MAKEINTRESOURCE(RT_DIALOG_VALUE)
#define MAKEINTRESOURCE(identifier) reinterpret_cast<LPCSTR>(static_cast<ULONG_PTR>(static_cast<WORD>(identifier)))
#define RGB(red, green, blue) (static_cast<COLORREF>(static_cast<BYTE>(red) | (static_cast<WORD>(static_cast<BYTE>(green)) << 8) | (static_cast<DWORD>(static_cast<BYTE>(blue)) << 16)))
#define MAKEWPARAM(low, high) static_cast<WPARAM>(static_cast<WORD>(low) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16))
#define MAKELPARAM(low, high) static_cast<LPARAM>(static_cast<WORD>(low) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16))
#define MAKEPOINTS(value) POINTS{static_cast<SHORT>(LOWORD(value)), static_cast<SHORT>(HIWORD(value))}
constexpr int SM_CXSCREEN = 0;
constexpr int SM_CYSCREEN = 1;
constexpr int SM_CXBORDER = 5;
constexpr int SM_CYBORDER = 6;
constexpr int SM_SWAPBUTTON = 23;
constexpr int SM_CXDRAG = 68;
constexpr int SM_CYDRAG = 69;
constexpr int SM_CXFULLSCREEN = 16;
constexpr int SM_CYFULLSCREEN = 17;
constexpr UINT MB_OK = 0x00000000;
constexpr UINT MB_OKCANCEL = 0x00000001;
constexpr UINT MB_YESNO = 0x00000004;
constexpr UINT MB_ICONERROR = 0x00000010;
constexpr UINT MB_ICONSTOP = MB_ICONERROR;
constexpr UINT MB_ICONQUESTION = 0x00000020;
constexpr UINT MB_ICONEXCLAMATION = 0x00000030;
constexpr UINT MB_ICONWARNING = MB_ICONEXCLAMATION;
constexpr UINT MB_SETFOREGROUND = 0x00010000;
constexpr UINT MB_TOPMOST = 0x00040000;
constexpr int IDYES = 6;
constexpr int IDNO = 7;
constexpr int IDABORT = 3;

struct MSGBOXPARAMS {
	UINT cbSize;
	HWND hwndOwner;
	HINSTANCE hInstance;
	LPCSTR lpszText;
	LPCSTR lpszCaption;
	DWORD dwStyle;
	LPCSTR lpszIcon;
	DWORD_PTR dwContextHelpId;
	void * lpfnMsgBoxCallback;
	DWORD dwLanguageId;
};

struct WIN32_FIND_DATAA {
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	char cFileName[MAX_PATH];
	char cAlternateFileName[14];
};
using WIN32_FIND_DATA = WIN32_FIND_DATAA;

struct BY_HANDLE_FILE_INFORMATION
{
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD dwVolumeSerialNumber;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD nNumberOfLinks;
	DWORD nFileIndexHigh;
	DWORD nFileIndexLow;
};

constexpr DWORD FILE_ATTRIBUTE_DIRECTORY = 0x10;
constexpr DWORD FILE_ATTRIBUTE_HIDDEN = 0x02;
constexpr DWORD FILE_ATTRIBUTE_SYSTEM = 0x04;
constexpr DWORD FILE_ATTRIBUTE_TEMPORARY = 0x100;
constexpr DWORD INVALID_FILE_ATTRIBUTES = 0xFFFFFFFFu;
constexpr DWORD GENERIC_READ = 0x80000000u;
constexpr DWORD GENERIC_WRITE = 0x40000000u;
constexpr DWORD FILE_SHARE_READ = 1;
constexpr DWORD FILE_SHARE_WRITE = 2;
constexpr DWORD CREATE_NEW = 1;
constexpr DWORD CREATE_ALWAYS = 2;
constexpr DWORD OPEN_EXISTING = 3;
constexpr DWORD OPEN_ALWAYS = 4;
constexpr DWORD FILE_ATTRIBUTE_NORMAL = 0x80;
constexpr DWORD FILE_FLAG_SEQUENTIAL_SCAN = 0x08000000;
constexpr DWORD FILE_BEGIN = SEEK_SET;
constexpr DWORD FILE_CURRENT = SEEK_CUR;
constexpr DWORD FILE_END = SEEK_END;
constexpr DWORD ERROR_ALREADY_EXISTS = 183;
constexpr DWORD ERROR_FILE_NOT_FOUND = 2;
constexpr DWORD ERROR_INVALID_HANDLE = 6;
constexpr DWORD WAIT_OBJECT_0 = 0;
constexpr DWORD WAIT_TIMEOUT = 258;
constexpr DWORD WAIT_FAILED = 0xFFFFFFFFu;
constexpr DWORD INFINITE = 0xFFFFFFFFu;
constexpr DWORD MUTEX_ALL_ACCESS = 0x001F0001;
constexpr DWORD STGM_SHARE_DENY_WRITE = 0x00000020L;
constexpr DWORD STGM_SHARE_EXCLUSIVE = 0x00000010L;
constexpr DWORD STGTY_STORAGE = 1;
constexpr DWORD STGTY_STREAM = 2;
constexpr DWORD STATFLAG_NONAME = 1;
constexpr DWORD CLSCTX_INPROC = 1;
constexpr ULONG PRSPEC_LPWSTR = 0;
constexpr ULONG PRSPEC_PROPID = 1;
constexpr DWORD PROPSETFLAG_DEFAULT = 0;
constexpr ULONG PID_FIRST_USABLE = 2;
constexpr VARTYPE VT_EMPTY = 0;
constexpr VARTYPE VT_I4 = 3;
constexpr VARTYPE VT_LPWSTR = 31;
constexpr VARTYPE VT_FILETIME = 64;

namespace OpenTSMacOS
{
	enum class HandleKind { File, Mutex };

	struct HandleBase
	{
		explicit HandleBase(HandleKind kind) : Kind(kind) {}
		virtual ~HandleBase() = default;
		HandleKind Kind;
	};

	struct FileHandle final : HandleBase
	{
		explicit FileHandle(int descriptor) : HandleBase(HandleKind::File), Descriptor(descriptor) {}
		~FileHandle() override { if (Descriptor >= 0) close(Descriptor); }
		int Descriptor;
	};

	// Named mutexes are backed by flock so ownership dies with the process,
	// matching Windows mutex semantics after a crash. PresenceDescriptor holds a
	// shared lock while any handle is open; OwnerDescriptor holds an exclusive
	// lock while the mutex is owned. Closing the descriptors releases both.
	struct MutexHandle final : HandleBase
	{
		MutexHandle() : HandleBase(HandleKind::Mutex) {}
		~MutexHandle() override
		{
			if (OwnerDescriptor >= 0) close(OwnerDescriptor);
			if (PresenceDescriptor >= 0) close(PresenceDescriptor);
		}
		std::recursive_timed_mutex Local;
		int PresenceDescriptor = -1;
		int OwnerDescriptor = -1;
	};

	inline thread_local DWORD LastError = 0;

	inline std::string NativePath(char const * path)
	{
		std::string result = path ? path : "";
		std::replace(result.begin(), result.end(), '\\', '/');
		return result;
	}

	inline std::string MutexLockPath(char const * name, char const * suffix)
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "/tmp/opents_%016zx.%s",
			std::hash<std::string_view>{}(name ? name : ""), suffix);
		return buffer;
	}

	/// <summary>
	/// Attaches lock files to the handle and reports whether another process
	/// already holds a handle to the same name via GetLastError:
	/// ERROR_ALREADY_EXISTS when live handles exist, 0 otherwise. When create is
	/// false and no live handle exists, fails with ERROR_FILE_NOT_FOUND.
	/// </summary>
	inline bool OpenNamedMutex(MutexHandle & handle, char const * name, bool create)
	{
		int const flags = O_RDWR | O_CLOEXEC | (create ? O_CREAT : 0);
		int const presence = open(MutexLockPath(name, "presence").c_str(), flags, 0666);
		if (presence < 0) { LastError = errno == ENOENT ? ERROR_FILE_NOT_FOUND : static_cast<DWORD>(errno); return false; }
		bool const existed = flock(presence, LOCK_EX | LOCK_NB) != 0;
		if (!existed && !create) { close(presence); LastError = ERROR_FILE_NOT_FOUND; return false; }
		flock(presence, LOCK_SH);
		int const owner = open(MutexLockPath(name, "owner").c_str(), O_RDWR | O_CLOEXEC | O_CREAT, 0666);
		if (owner < 0) { LastError = static_cast<DWORD>(errno); close(presence); return false; }
		handle.PresenceDescriptor = presence;
		handle.OwnerDescriptor = owner;
		LastError = existed ? ERROR_ALREADY_EXISTS : 0;
		return true;
	}
}

inline void SetLastError(DWORD error) { OpenTSMacOS::LastError = error; }
inline DWORD GetLastError() { return OpenTSMacOS::LastError; }

inline HANDLE CreateFile(char const * path, DWORD access, DWORD, void *, DWORD disposition, DWORD, HANDLE)
{
	int flags = access == GENERIC_READ ? O_RDONLY
		: access == GENERIC_WRITE ? O_WRONLY : O_RDWR;
	switch (disposition) {
		case CREATE_NEW: flags |= O_CREAT | O_EXCL; break;
		case CREATE_ALWAYS: flags |= O_CREAT | O_TRUNC; break;
		case OPEN_ALWAYS: flags |= O_CREAT; break;
		case OPEN_EXISTING: break;
		default: SetLastError(EINVAL); return INVALID_HANDLE_VALUE;
	}
	std::string const native = OpenTSMacOS::NativePath(path);
	int const descriptor = open(native.c_str(), flags, 0666);
	if (descriptor < 0) {
		SetLastError(errno);
		return INVALID_HANDLE_VALUE;
	}
	SetLastError(0);
	return new OpenTSMacOS::FileHandle(descriptor);
}

inline BOOL ReadFile(HANDLE handle, void * buffer, DWORD size, DWORD * actual, void *)
{
	if (actual) *actual = 0;
	if (!handle || handle == INVALID_HANDLE_VALUE) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
	auto * base = static_cast<OpenTSMacOS::HandleBase *>(handle);
	if (base->Kind != OpenTSMacOS::HandleKind::File) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
	ssize_t const result = read(static_cast<OpenTSMacOS::FileHandle *>(base)->Descriptor, buffer, size);
	if (result < 0) { SetLastError(errno); return FALSE; }
	if (actual) *actual = static_cast<DWORD>(result);
	return TRUE;
}

inline BOOL WriteFile(HANDLE handle, void const * buffer, DWORD size, DWORD * actual, void *)
{
	if (actual) *actual = 0;
	if (!handle || handle == INVALID_HANDLE_VALUE) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
	auto * base = static_cast<OpenTSMacOS::HandleBase *>(handle);
	if (base->Kind != OpenTSMacOS::HandleKind::File) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
	ssize_t const result = write(static_cast<OpenTSMacOS::FileHandle *>(base)->Descriptor, buffer, size);
	if (result < 0) { SetLastError(errno); return FALSE; }
	if (actual) *actual = static_cast<DWORD>(result);
	return TRUE;
}

inline DWORD SetFilePointer(HANDLE handle, LONG distance, LONG *, DWORD origin)
{
	if (!handle || handle == INVALID_HANDLE_VALUE) { SetLastError(ERROR_INVALID_HANDLE); return 0xFFFFFFFFu; }
	auto * base = static_cast<OpenTSMacOS::HandleBase *>(handle);
	if (base->Kind != OpenTSMacOS::HandleKind::File) { SetLastError(ERROR_INVALID_HANDLE); return 0xFFFFFFFFu; }
	off_t const result = lseek(static_cast<OpenTSMacOS::FileHandle *>(base)->Descriptor, distance, static_cast<int>(origin));
	if (result < 0 || static_cast<std::uint64_t>(result) > 0xFFFFFFFFu) {
		SetLastError(result < 0 ? errno : EOVERFLOW);
		return 0xFFFFFFFFu;
	}
	return static_cast<DWORD>(result);
}

inline DWORD GetFileSize(HANDLE handle, DWORD * high)
{
	if (high) *high = 0;
	if (!handle || handle == INVALID_HANDLE_VALUE) { SetLastError(ERROR_INVALID_HANDLE); return 0xFFFFFFFFu; }
	auto * base = static_cast<OpenTSMacOS::HandleBase *>(handle);
	if (base->Kind != OpenTSMacOS::HandleKind::File) { SetLastError(ERROR_INVALID_HANDLE); return 0xFFFFFFFFu; }
	struct stat info = {};
	if (fstat(static_cast<OpenTSMacOS::FileHandle *>(base)->Descriptor, &info) != 0) {
		SetLastError(errno);
		return 0xFFFFFFFFu;
	}
	if (high) *high = static_cast<DWORD>(static_cast<std::uint64_t>(info.st_size) >> 32);
	return static_cast<DWORD>(info.st_size);
}

inline ULONGLONG OpenTSMacOSFileTime(std::time_t seconds, long nanoseconds = 0)
{
	constexpr ULONGLONG UnixEpochInFileTime = 11644473600ULL * 10000000ULL;
	return UnixEpochInFileTime + static_cast<ULONGLONG>(seconds) * 10000000ULL
		+ static_cast<ULONGLONG>(nanoseconds / 100);
}

inline FILETIME OpenTSMacOSFileTime(std::time_t seconds, long nanoseconds, int)
{
	ULONGLONG const stamp = OpenTSMacOSFileTime(seconds, nanoseconds);
	return {static_cast<DWORD>(stamp), static_cast<DWORD>(stamp >> 32)};
}

inline BOOL GetFileInformationByHandle(HANDLE handle, BY_HANDLE_FILE_INFORMATION * information)
{
	if (!handle || handle == INVALID_HANDLE_VALUE || information == nullptr) return FALSE;
	auto * base = static_cast<OpenTSMacOS::HandleBase *>(handle);
	if (base->Kind != OpenTSMacOS::HandleKind::File) return FALSE;
	struct stat info = {};
	if (fstat(static_cast<OpenTSMacOS::FileHandle *>(base)->Descriptor, &info) != 0) return FALSE;
	memset(information, 0, sizeof(*information));
	information->dwFileAttributes = S_ISDIR(info.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	information->ftCreationTime = OpenTSMacOSFileTime(info.st_birthtimespec.tv_sec, info.st_birthtimespec.tv_nsec, 0);
	information->ftLastAccessTime = OpenTSMacOSFileTime(info.st_atimespec.tv_sec, info.st_atimespec.tv_nsec, 0);
	information->ftLastWriteTime = OpenTSMacOSFileTime(info.st_mtimespec.tv_sec, info.st_mtimespec.tv_nsec, 0);
	information->nFileSizeHigh = static_cast<DWORD>(static_cast<ULONGLONG>(info.st_size) >> 32);
	information->nFileSizeLow = static_cast<DWORD>(info.st_size);
	information->nNumberOfLinks = static_cast<DWORD>(info.st_nlink);
	information->nFileIndexHigh = static_cast<DWORD>(static_cast<ULONGLONG>(info.st_ino) >> 32);
	information->nFileIndexLow = static_cast<DWORD>(info.st_ino);
	return TRUE;
}

inline BOOL GetFileTime(HANDLE handle, FILETIME * created, FILETIME * accessed, FILETIME * written)
{
	BY_HANDLE_FILE_INFORMATION information = {};
	if (!GetFileInformationByHandle(handle, &information)) return FALSE;
	if (created) *created = information.ftCreationTime;
	if (accessed) *accessed = information.ftLastAccessTime;
	if (written) *written = information.ftLastWriteTime;
	return TRUE;
}

inline std::time_t OpenTSMacOSUnixTime(FILETIME const & time)
{
	constexpr ULONGLONG UnixEpochInFileTime = 11644473600ULL * 10000000ULL;
	ULONGLONG const stamp = static_cast<ULONGLONG>(time.dwLowDateTime)
		| (static_cast<ULONGLONG>(time.dwHighDateTime) << 32);
	return stamp < UnixEpochInFileTime ? 0 : static_cast<std::time_t>((stamp - UnixEpochInFileTime) / 10000000ULL);
}

inline BOOL FileTimeToDosDateTime(FILETIME const * time, WORD * date, WORD * clock)
{
	if (!time || !date || !clock) return FALSE;
	std::time_t const stamp = OpenTSMacOSUnixTime(*time);
	std::tm local = {};
	if (localtime_r(&stamp, &local) == nullptr || local.tm_year < 80 || local.tm_year > 207) return FALSE;
	*date = static_cast<WORD>(((local.tm_year - 80) << 9) | ((local.tm_mon + 1) << 5) | local.tm_mday);
	*clock = static_cast<WORD>((local.tm_hour << 11) | (local.tm_min << 5) | (local.tm_sec / 2));
	return TRUE;
}

inline BOOL DosDateTimeToFileTime(WORD date, WORD clock, FILETIME * time)
{
	if (!time) return FALSE;
	std::tm local = {};
	local.tm_year = ((date >> 9) & 0x7F) + 80;
	local.tm_mon = ((date >> 5) & 0x0F) - 1;
	local.tm_mday = date & 0x1F;
	local.tm_hour = (clock >> 11) & 0x1F;
	local.tm_min = (clock >> 5) & 0x3F;
	local.tm_sec = (clock & 0x1F) * 2;
	std::time_t const stamp = mktime(&local);
	if (stamp == static_cast<std::time_t>(-1)) return FALSE;
	*time = OpenTSMacOSFileTime(stamp, 0, 0);
	return TRUE;
}

inline void GetSystemTime(SYSTEMTIME * time)
{
	if (!time) return;
	auto const now = std::chrono::system_clock::now();
	auto const seconds = std::chrono::system_clock::to_time_t(now);
	std::tm utc = {};
	gmtime_r(&seconds, &utc);
	time->wYear = static_cast<WORD>(utc.tm_year + 1900);
	time->wMonth = static_cast<WORD>(utc.tm_mon + 1);
	time->wDayOfWeek = static_cast<WORD>(utc.tm_wday);
	time->wDay = static_cast<WORD>(utc.tm_mday);
	time->wHour = static_cast<WORD>(utc.tm_hour);
	time->wMinute = static_cast<WORD>(utc.tm_min);
	time->wSecond = static_cast<WORD>(utc.tm_sec);
	time->wMilliseconds = static_cast<WORD>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000);
}

inline BOOL SystemTimeToFileTime(SYSTEMTIME const * time, FILETIME * result)
{
	if (!time || !result) return(FALSE);
	std::tm utc = {};
	utc.tm_year = time->wYear - 1900;
	utc.tm_mon = time->wMonth - 1;
	utc.tm_mday = time->wDay;
	utc.tm_hour = time->wHour;
	utc.tm_min = time->wMinute;
	utc.tm_sec = time->wSecond;
	std::time_t const stamp = timegm(&utc);
	if (stamp == static_cast<std::time_t>(-1)) return(FALSE);
	*result = OpenTSMacOSFileTime(stamp, static_cast<long>(time->wMilliseconds) * 1000000L, 0);
	return(TRUE);
}

inline BOOL FileTimeToSystemTime(FILETIME const * time, SYSTEMTIME * result)
{
	if (!time || !result) return(FALSE);
	std::time_t const stamp = OpenTSMacOSUnixTime(*time);
	std::tm utc = {};
	if (!gmtime_r(&stamp, &utc)) return(FALSE);
	result->wYear = static_cast<WORD>(utc.tm_year + 1900);
	result->wMonth = static_cast<WORD>(utc.tm_mon + 1);
	result->wDayOfWeek = static_cast<WORD>(utc.tm_wday);
	result->wDay = static_cast<WORD>(utc.tm_mday);
	result->wHour = static_cast<WORD>(utc.tm_hour);
	result->wMinute = static_cast<WORD>(utc.tm_min);
	result->wSecond = static_cast<WORD>(utc.tm_sec);
	result->wMilliseconds = 0;
	return(TRUE);
}

inline BOOL FileTimeToLocalFileTime(FILETIME const * source, FILETIME * destination)
{
	if (!source || !destination) return(FALSE);
	*destination = *source;
	return(TRUE);
}

inline int GetDateFormat(LCID, DWORD, SYSTEMTIME const * time, LPCSTR, LPSTR buffer, int size)
{
	if (!time || !buffer || size <= 0) return(0);
	return(std::snprintf(buffer, size, "%04u-%02u-%02u", time->wYear, time->wMonth, time->wDay) + 1);
}

inline int GetTimeFormat(LCID, DWORD, SYSTEMTIME const * time, LPCSTR, LPSTR buffer, int size)
{
	if (!time || !buffer || size <= 0) return(0);
	return(std::snprintf(buffer, size, "%02u:%02u", time->wHour, time->wMinute) + 1);
}

inline BOOL SetFileTime(HANDLE handle, FILETIME const *, FILETIME const * accessed, FILETIME const * written)
{
	if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
	auto * base = static_cast<OpenTSMacOS::HandleBase *>(handle);
	if (base->Kind != OpenTSMacOS::HandleKind::File) return FALSE;
	struct stat info = {};
	int const descriptor = static_cast<OpenTSMacOS::FileHandle *>(base)->Descriptor;
	if (fstat(descriptor, &info) != 0) return FALSE;
	struct timespec times[2] = {info.st_atimespec, info.st_mtimespec};
	if (accessed) times[0] = {OpenTSMacOSUnixTime(*accessed), 0};
	if (written) times[1] = {OpenTSMacOSUnixTime(*written), 0};
	return futimens(descriptor, times) == 0;
}

inline BOOL CloseHandle(HANDLE handle)
{
	if (!handle || handle == INVALID_HANDLE_VALUE) { SetLastError(ERROR_INVALID_HANDLE); return FALSE; }
	delete static_cast<OpenTSMacOS::HandleBase *>(handle);
	return TRUE;
}

inline BOOL DeleteFile(char const * path)
{
	std::string const native = OpenTSMacOS::NativePath(path);
	if (unlink(native.c_str()) == 0) return TRUE;
	SetLastError(errno);
	return FALSE;
}

inline LONG CompareFileTime(FILETIME const * left, FILETIME const * right)
{
	if (!left || !right) return(0);
	ULONGLONG const left_stamp = static_cast<ULONGLONG>(left->dwLowDateTime)
		| (static_cast<ULONGLONG>(left->dwHighDateTime) << 32);
	ULONGLONG const right_stamp = static_cast<ULONGLONG>(right->dwLowDateTime)
		| (static_cast<ULONGLONG>(right->dwHighDateTime) << 32);
	return(left_stamp < right_stamp ? -1 : left_stamp > right_stamp ? 1 : 0);
}

inline BOOL CopyFile(char const * source, char const * destination, BOOL fail_if_exists)
{
	std::string const native_source = OpenTSMacOS::NativePath(source);
	std::string const native_destination = OpenTSMacOS::NativePath(destination);
	int const input = open(native_source.c_str(), O_RDONLY);
	if (input < 0) { SetLastError(errno); return(FALSE); }
	int flags = O_WRONLY | O_CREAT | O_TRUNC;
	if (fail_if_exists) flags = O_WRONLY | O_CREAT | O_EXCL;
	int const output = open(native_destination.c_str(), flags, 0666);
	if (output < 0) { SetLastError(errno); close(input); return(FALSE); }
	char buffer[65536];
	bool succeeded = true;
	for (;;) {
		ssize_t const bytes = read(input, buffer, sizeof(buffer));
		if (bytes == 0) break;
		if (bytes < 0) { succeeded = false; break; }
		ssize_t offset = 0;
		while (offset < bytes) {
			ssize_t const written = write(output, buffer + offset, static_cast<size_t>(bytes - offset));
			if (written <= 0) { succeeded = false; break; }
			offset += written;
		}
		if (!succeeded) break;
	}
	if (!succeeded) SetLastError(errno);
	close(output);
	close(input);
	return(succeeded ? TRUE : FALSE);
}

inline DWORD GetFileAttributes(char const * path)
{
	std::string const native = OpenTSMacOS::NativePath(path);
	struct stat info = {};
	if (stat(native.c_str(), &info) != 0) { SetLastError(errno); return INVALID_FILE_ATTRIBUTES; }
	DWORD attributes = S_ISDIR(info.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
	char const * name = std::strrchr(native.c_str(), '/');
	name = name ? name + 1 : native.c_str();
	if (*name == '.') attributes |= FILE_ATTRIBUTE_HIDDEN;
	return attributes;
}

inline BOOL CreateDirectory(char const * path, void *)
{
	std::string const native = OpenTSMacOS::NativePath(path);
	if (mkdir(native.c_str(), 0777) == 0) return TRUE;
	SetLastError(errno == EEXIST ? ERROR_ALREADY_EXISTS : static_cast<DWORD>(errno));
	return FALSE;
}

inline BOOL SetCurrentDirectory(char const * path)
{
	std::string const native = OpenTSMacOS::NativePath(path);
	if (chdir(native.empty() ? "." : native.c_str()) == 0) return(TRUE);
	SetLastError(errno);
	return(FALSE);
}

inline HANDLE CreateMutex(void *, BOOL initially_owned, char const * name)
{
	auto * handle = new OpenTSMacOS::MutexHandle;
	if (name && *name) {
		if (!OpenTSMacOS::OpenNamedMutex(*handle, name, true)) { delete handle; return nullptr; }
		if (initially_owned) flock(handle->OwnerDescriptor, LOCK_EX);
	} else {
		SetLastError(0);
		if (initially_owned) handle->Local.lock();
	}
	return handle;
}

inline HANDLE OpenMutex(DWORD, BOOL, char const * name)
{
	auto * handle = new OpenTSMacOS::MutexHandle;
	if (!OpenTSMacOS::OpenNamedMutex(*handle, name, false)) { delete handle; return nullptr; }
	return handle;
}

inline DWORD WaitForSingleObject(HANDLE raw_handle, DWORD timeout)
{
	if (!raw_handle || raw_handle == INVALID_HANDLE_VALUE) return WAIT_FAILED;
	auto * handle = static_cast<OpenTSMacOS::HandleBase *>(raw_handle);
	if (handle->Kind != OpenTSMacOS::HandleKind::Mutex) return WAIT_FAILED;
	auto * mutex = static_cast<OpenTSMacOS::MutexHandle *>(handle);
	if (mutex->OwnerDescriptor >= 0) {
		if (timeout == INFINITE) return flock(mutex->OwnerDescriptor, LOCK_EX) == 0 ? WAIT_OBJECT_0 : WAIT_FAILED;
		auto const deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
		do {
			if (flock(mutex->OwnerDescriptor, LOCK_EX | LOCK_NB) == 0) return WAIT_OBJECT_0;
			if (errno != EWOULDBLOCK && errno != EINTR) return WAIT_FAILED;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		} while (std::chrono::steady_clock::now() < deadline);
		return WAIT_TIMEOUT;
	}
	if (timeout == INFINITE) { mutex->Local.lock(); return WAIT_OBJECT_0; }
	return mutex->Local.try_lock_for(std::chrono::milliseconds(timeout)) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
}

inline BOOL ReleaseMutex(HANDLE raw_handle)
{
	if (!raw_handle || raw_handle == INVALID_HANDLE_VALUE) return FALSE;
	auto * handle = static_cast<OpenTSMacOS::HandleBase *>(raw_handle);
	if (handle->Kind != OpenTSMacOS::HandleKind::Mutex) return FALSE;
	auto * mutex = static_cast<OpenTSMacOS::MutexHandle *>(handle);
	if (mutex->OwnerDescriptor >= 0) return flock(mutex->OwnerDescriptor, LOCK_UN) == 0;
	mutex->Local.unlock();
	return TRUE;
}

inline DWORD WaitForMultipleObjects(DWORD count, HANDLE const * handles, BOOL wait_all, DWORD timeout)
{
	if (!wait_all) {
		for (DWORD index = 0; index < count; ++index) {
			if (WaitForSingleObject(handles[index], 0) == WAIT_OBJECT_0) return WAIT_OBJECT_0 + index;
		}
		return WAIT_TIMEOUT;
	}
	auto const deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
	DWORD acquired = 0;
	for (; acquired < count; ++acquired) {
		DWORD remaining = timeout;
		if (timeout != INFINITE) {
			auto const now = std::chrono::steady_clock::now();
			if (now >= deadline) break;
			remaining = static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
		}
		if (WaitForSingleObject(handles[acquired], remaining) != WAIT_OBJECT_0) break;
	}
	if (acquired == count) return WAIT_OBJECT_0;
	while (acquired != 0) ReleaseMutex(handles[--acquired]);
	return WAIT_TIMEOUT;
}

struct _GUID {
	std::uint32_t Data1;
	std::uint16_t Data2;
	std::uint16_t Data3;
	std::uint8_t Data4[8];
};
using GUID = _GUID;
using IID = GUID;
using CLSID = GUID;
using FMTID = GUID;
#define __IID_DEFINED__
#define CLSID_DEFINED
using REFGUID = GUID const &;
using REFIID = IID const &;
using REFCLSID = CLSID const &;
using REFFMTID = FMTID const &;

inline bool operator==(GUID const & left, GUID const & right)
{
	return(std::memcmp(&left, &right, sizeof(GUID)) == 0);
}

inline bool operator!=(GUID const & left, GUID const & right)
{
	return(!(left == right));
}

inline constexpr GUID IID_IUnknown = {0x00000000, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr GUID IID_IStream = {0x0000000C, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr GUID IID_IStorage = {0x0000000B, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr GUID IID_IPersist = {0x0000010C, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr GUID IID_IPersistStream = {0x00000109, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr GUID IID_IClassFactory = {0x00000001, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr GUID IID_IPropertyStorage = {0x00000138, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr GUID IID_IPropertySetStorage = {0x0000013A, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46}};
inline constexpr FMTID FMTID_SummaryInformation = {
	0xF29F85E0, 0x4FF9, 0x1068, {0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9}};

struct STATSTG {
	LPOLESTR pwcsName;
	DWORD type;
	ULARGE_INTEGER cbSize;
	FILETIME mtime;
	FILETIME ctime;
	FILETIME atime;
	DWORD grfMode;
	DWORD grfLocksSupported;
	CLSID clsid;
	DWORD grfStateBits;
	DWORD reserved;
};

using PROPID = ULONG;
using SNB = LPOLESTR *;
struct IUnknown;
struct IStream;
struct IStorage;
struct IEnumSTATSTG;
struct IEnumSTATPROPSTG;
struct IEnumSTATPROPSETSTG;

struct PROPSPEC {
	ULONG ulKind;
	union {
		PROPID propid;
		LPOLESTR lpwstr;
	};
};

struct PROPVARIANT {
	VARTYPE vt;
	WORD wReserved1;
	WORD wReserved2;
	WORD wReserved3;
	union {
		LONG lVal;
		ULONG ulVal;
		FILETIME filetime;
		LPSTR pszVal;
		LPWSTR pwszVal;
		IUnknown * punkVal;
		IStream * pStream;
		IStorage * pStorage;
	};
};

struct STATPROPSETSTG {
	FMTID fmtid;
	CLSID clsid;
	DWORD grfFlags;
	FILETIME mtime;
	FILETIME ctime;
	FILETIME atime;
	DWORD dwOSVersion;
};

struct DECLSPEC_UUID("00000000-0000-0000-C000-000000000046") IUnknown {
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void **) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
	protected: virtual ~IUnknown(void) = default;
};

struct DECLSPEC_UUID("0C733A30-2A1C-11CE-ADE5-00AA0044773D") ISequentialStream : IUnknown {
	virtual HRESULT STDMETHODCALLTYPE Read(void *, ULONG, ULONG *) = 0;
	virtual HRESULT STDMETHODCALLTYPE Write(void const *, ULONG, ULONG *) = 0;
};

struct DECLSPEC_UUID("0000000C-0000-0000-C000-000000000046") IStream : ISequentialStream {
	virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) = 0;
	virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) = 0;
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE Revert(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *, DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE Clone(IStream **) = 0;
};

struct DECLSPEC_UUID("0000000B-0000-0000-C000-000000000046") IStorage : IUnknown {
	virtual HRESULT STDMETHODCALLTYPE CreateStream(WCHAR const *, DWORD, DWORD, DWORD, IStream **) = 0;
	virtual HRESULT STDMETHODCALLTYPE OpenStream(WCHAR const *, void *, DWORD, DWORD, IStream **) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateStorage(WCHAR const *, DWORD, DWORD, DWORD, IStorage **) = 0;
	virtual HRESULT STDMETHODCALLTYPE OpenStorage(WCHAR const *, IStorage *, DWORD, SNB, DWORD, IStorage **) = 0;
	virtual HRESULT STDMETHODCALLTYPE CopyTo(DWORD, IID const *, SNB, IStorage *) = 0;
	virtual HRESULT STDMETHODCALLTYPE MoveElementTo(WCHAR const *, IStorage *, WCHAR const *, DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE Revert(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnumElements(DWORD, void *, DWORD, IEnumSTATSTG **) = 0;
	virtual HRESULT STDMETHODCALLTYPE DestroyElement(WCHAR const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE RenameElement(WCHAR const *, WCHAR const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetElementTimes(WCHAR const *, FILETIME const *, FILETIME const *, FILETIME const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetClass(REFCLSID) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetStateBits(DWORD, DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *, DWORD) = 0;
};

struct DECLSPEC_UUID("00000138-0000-0000-C000-000000000046") IPropertyStorage : IUnknown {
	virtual HRESULT STDMETHODCALLTYPE ReadMultiple(ULONG, PROPSPEC const *, PROPVARIANT *) = 0;
	virtual HRESULT STDMETHODCALLTYPE WriteMultiple(ULONG, PROPSPEC const *, PROPVARIANT const *, PROPID) = 0;
	virtual HRESULT STDMETHODCALLTYPE DeleteMultiple(ULONG, PROPSPEC const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReadPropertyNames(ULONG, PROPID const *, LPOLESTR *) = 0;
	virtual HRESULT STDMETHODCALLTYPE WritePropertyNames(ULONG, PROPID const *, LPOLESTR const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE DeletePropertyNames(ULONG, PROPID const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE Commit(DWORD) = 0;
	virtual HRESULT STDMETHODCALLTYPE Revert(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE Enum(IEnumSTATPROPSTG **) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetTimes(FILETIME const *, FILETIME const *, FILETIME const *) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetClass(REFCLSID) = 0;
	virtual HRESULT STDMETHODCALLTYPE Stat(STATPROPSETSTG *) = 0;
};

struct DECLSPEC_UUID("0000013A-0000-0000-C000-000000000046") IPropertySetStorage : IUnknown {
	virtual HRESULT STDMETHODCALLTYPE Create(REFFMTID, CLSID const *, DWORD, DWORD, IPropertyStorage **) = 0;
	virtual HRESULT STDMETHODCALLTYPE Open(REFFMTID, DWORD, IPropertyStorage **) = 0;
	virtual HRESULT STDMETHODCALLTYPE Delete(REFFMTID) = 0;
	virtual HRESULT STDMETHODCALLTYPE Enum(IEnumSTATPROPSETSTG **) = 0;
};

struct DECLSPEC_UUID("0000010C-0000-0000-C000-000000000046") IPersist : IUnknown {
	virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID *) = 0;
};

struct DECLSPEC_UUID("00000109-0000-0000-C000-000000000046") IPersistStream : IPersist {
	virtual HRESULT STDMETHODCALLTYPE IsDirty(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE Load(IStream *) = 0;
	virtual HRESULT STDMETHODCALLTYPE Save(IStream *, BOOL) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetSizeMax(ULARGE_INTEGER *) = 0;
};

struct DECLSPEC_UUID("00000001-0000-0000-C000-000000000046") IClassFactory : IUnknown {
	virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *, REFIID, void **) = 0;
	virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL) = 0;
};

HRESULT CoCreateInstance(REFCLSID class_id, IUnknown * outer, DWORD context, REFIID interface_id, void ** object);
HRESULT CoRegisterClassObject(REFCLSID class_id, IUnknown * factory, DWORD context, DWORD flags, DWORD * registration);
HRESULT CoRevokeClassObject(DWORD registration);

template<class Interface, GUID const * Identifier>
struct _com_IIID {
	using InterfaceType = Interface;
	static constexpr GUID const * InterfaceIdentifier = Identifier;
};

template<class Descriptor>
class _com_ptr_t
{
	public:
		using Interface = typename Descriptor::InterfaceType;
		_com_ptr_t(void) : Pointer(nullptr) {}
		_com_ptr_t(Interface * pointer, bool addref = true) : Pointer(pointer) { if (Pointer && addref) Pointer->AddRef(); }
		template<class Other>
		_com_ptr_t(Other * source) : Pointer(nullptr)
		{
			if (source) source->QueryInterface(*Descriptor::InterfaceIdentifier, reinterpret_cast<void **>(&Pointer));
		}
		_com_ptr_t(REFCLSID class_id, IUnknown * outer = nullptr, DWORD context = 0) : Pointer(nullptr)
		{
			CreateInstance(class_id, outer, context);
		}
		_com_ptr_t(_com_ptr_t const & that) : Pointer(that.Pointer) { if (Pointer) Pointer->AddRef(); }
		template<class OtherDescriptor>
		_com_ptr_t(_com_ptr_t<OtherDescriptor> const & that) : Pointer(nullptr)
		{
			IUnknown * source = that.GetInterface();
			if (source) source->QueryInterface(*Descriptor::InterfaceIdentifier, reinterpret_cast<void **>(&Pointer));
		}
		~_com_ptr_t(void) { Release(); }
		_com_ptr_t & operator=(_com_ptr_t const & that) { if (this != &that) Attach(that.Pointer); return(*this); }
		template<class OtherDescriptor>
		_com_ptr_t & operator=(_com_ptr_t<OtherDescriptor> const & that)
		{
			Release();
			IUnknown * source = that.GetInterface();
			if (source) source->QueryInterface(*Descriptor::InterfaceIdentifier, reinterpret_cast<void **>(&Pointer));
			return(*this);
		}
		Interface * operator->(void) const { return(Pointer); }
		operator Interface *(void) const { return(Pointer); }
		Interface * GetInterface(void) const { return(Pointer); }
		Interface ** operator&(void) { Release(); return(&Pointer); }
		void Attach(Interface * pointer, bool addref = true) { Release(); Pointer = pointer; if (Pointer && addref) Pointer->AddRef(); }
		Interface * Detach(void) { Interface * result = Pointer; Pointer = nullptr; return(result); }
		void Release(void) { if (Pointer) { Interface * old = Pointer; Pointer = nullptr; old->Release(); } }
		HRESULT CreateInstance(REFCLSID class_id, IUnknown * outer = nullptr, DWORD context = 0)
		{
			Release();
			return(CoCreateInstance(class_id, outer, context, *Descriptor::InterfaceIdentifier,
				reinterpret_cast<void **>(&Pointer)));
		}
	private:
		Interface * Pointer;
};

#define _COM_SMARTPTR_TYPEDEF(Interface, iid) \
	typedef _com_ptr_t<_com_IIID<Interface, &iid>> Interface##Ptr

_COM_SMARTPTR_TYPEDEF(IUnknown, IID_IUnknown);
_COM_SMARTPTR_TYPEDEF(IStream, IID_IStream);
_COM_SMARTPTR_TYPEDEF(IStorage, IID_IStorage);
_COM_SMARTPTR_TYPEDEF(IPersist, IID_IPersist);
_COM_SMARTPTR_TYPEDEF(IPersistStream, IID_IPersistStream);
_COM_SMARTPTR_TYPEDEF(IPropertyStorage, IID_IPropertyStorage);
_COM_SMARTPTR_TYPEDEF(IPropertySetStorage, IID_IPropertySetStorage);

using LPPERSISTSTREAM = IPersistStream *;

HRESULT StgCreateDocfile(WCHAR const * path, DWORD mode, DWORD reserved, IStorage ** storage);
HRESULT StgOpenStorage(WCHAR const * path, IStorage * priority, DWORD mode, SNB exclude, DWORD reserved,
	IStorage ** storage);
HRESULT PropVariantClear(PROPVARIANT * value);
HRESULT CoFileTimeNow(FILETIME * time);

inline void _com_issue_error(HRESULT result)
{
	throw std::runtime_error("COM operation failed: " + std::to_string(result));
}

inline HRESULT OleSaveToStream(IPersistStream * persistent, IStream * stream)
{
	if (!persistent || !stream) return(E_POINTER);
	CLSID class_id = {};
	HRESULT result = persistent->GetClassID(&class_id);
	if (FAILED(result)) return(result);
	ULONG written = 0;
	result = stream->Write(&class_id, sizeof(class_id), &written);
	if (FAILED(result) || written != sizeof(class_id)) return(FAILED(result) ? result : E_FAIL);
	return(persistent->Save(stream, TRUE));
}

inline HRESULT OleLoadFromStream(IStream * stream, REFIID interface_id, void ** object)
{
	if (!stream || !object) return(E_POINTER);
	*object = nullptr;
	CLSID class_id = {};
	ULONG read = 0;
	HRESULT result = stream->Read(&class_id, sizeof(class_id), &read);
	if (FAILED(result) || read != sizeof(class_id)) return(FAILED(result) ? result : E_FAIL);
	IPersistStream * persistent = nullptr;
	result = CoCreateInstance(class_id, nullptr, 0, IID_IPersistStream, reinterpret_cast<void **>(&persistent));
	if (FAILED(result)) return(result);
	result = persistent->Load(stream);
	if (SUCCEEDED(result)) result = persistent->QueryInterface(interface_id, object);
	persistent->Release();
	return(result);
}

struct CRITICAL_SECTION { std::recursive_mutex Mutex; };
inline void InitializeCriticalSection(CRITICAL_SECTION *) {}
inline void DeleteCriticalSection(CRITICAL_SECTION *) {}
inline void EnterCriticalSection(CRITICAL_SECTION * section) { section->Mutex.lock(); }
inline void LeaveCriticalSection(CRITICAL_SECTION * section) { section->Mutex.unlock(); }
inline LONG InterlockedIncrement(LONG volatile * value) { return(__sync_add_and_fetch(value, 1)); }
inline LONG InterlockedDecrement(LONG volatile * value) { return(__sync_sub_and_fetch(value, 1)); }

inline DWORD timeGetTime(void)
{
	auto const now = std::chrono::steady_clock::now().time_since_epoch();
	return(static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count()));
}
inline DWORD GetTickCount(void) { return(timeGetTime()); }
inline DWORD GetCurrentThreadId(void)
{
	return(static_cast<DWORD>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
}
inline void Sleep(DWORD milliseconds) { std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }
inline BOOL QueryPerformanceCounter(LARGE_INTEGER * value)
{
	value->QuadPart = std::chrono::steady_clock::now().time_since_epoch().count();
	return(TRUE);
}

using LPTIMECALLBACK = void (CALLBACK *)(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
constexpr MMRESULT TIMERR_NOCANDO = 97;
constexpr UINT TIME_PERIODIC = 1;

namespace OpenTSMacOS
{
	struct TimerState
	{
		std::mutex Mutex;
		std::condition_variable Condition;
		bool Cancelled = false;
		std::thread Thread;
	};

	inline std::mutex TimerMutex;
	inline std::unordered_map<UINT, TimerState *> Timers;
	inline std::atomic<UINT> NextTimer {1};
}

inline MMRESULT timeBeginPeriod(UINT) { return(0); }
inline MMRESULT timeEndPeriod(UINT) { return(0); }
inline UINT timeSetEvent(UINT delay, UINT, LPTIMECALLBACK callback, DWORD_PTR user, UINT mode)
{
	using namespace OpenTSMacOS;
	if (!callback || delay == 0) return(0);

	UINT const identifier = NextTimer.fetch_add(1);
	auto * state = new TimerState;
	{
		std::lock_guard lock(TimerMutex);
		Timers.emplace(identifier, state);
	}
	state->Thread = std::thread([state, identifier, delay, callback, user, mode]() {
		std::unique_lock lock(state->Mutex);
		do {
			if (state->Condition.wait_for(lock, std::chrono::milliseconds(delay), [state]() { return(state->Cancelled); })) break;
			lock.unlock();
			callback(identifier, 0, user, 0, 0);
			lock.lock();
		} while (mode & TIME_PERIODIC);
	});
	return(identifier);
}

inline MMRESULT timeKillEvent(UINT identifier)
{
	using namespace OpenTSMacOS;
	TimerState * state = nullptr;
	{
		std::lock_guard lock(TimerMutex);
		auto const timer = Timers.find(identifier);
		if (timer == Timers.end()) return(TIMERR_NOCANDO);
		state = timer->second;
		Timers.erase(timer);
	}
	{
		std::lock_guard lock(state->Mutex);
		state->Cancelled = true;
	}
	state->Condition.notify_all();
	if (state->Thread.joinable()) state->Thread.join();
	delete state;
	return(0);
}
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER * value)
{
	using Period = std::chrono::steady_clock::period;
	value->QuadPart = Period::den / Period::num;
	return(TRUE);
}

#define LOWORD(value) static_cast<WORD>(static_cast<DWORD_PTR>(value) & 0xFFFFu)
#define HIWORD(value) static_cast<WORD>((static_cast<DWORD_PTR>(value) >> 16) & 0xFFFFu)
#define LOBYTE(value) static_cast<BYTE>(static_cast<WORD>(value) & 0xFFu)
#define HIBYTE(value) static_cast<BYTE>((static_cast<WORD>(value) >> 8) & 0xFFu)
#define MAKELONG(low, high) static_cast<LONG>((static_cast<WORD>(low)) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16))
#define MAKEWORD(low, high) static_cast<WORD>((static_cast<BYTE>(low)) | (static_cast<WORD>(static_cast<BYTE>(high)) << 8))
#define RT_STRING MAKEINTRESOURCE(6)

BOOL PostMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

namespace OpenTSMacOS
{
	inline std::mutex SocketWatchMutex;
	inline std::unordered_map<SOCKET, std::shared_ptr<std::atomic_bool>> SocketWatches;

	inline void CancelSocketWatch(SOCKET socket)
	{
		std::lock_guard lock(SocketWatchMutex);
		auto const found = SocketWatches.find(socket);
		if (found != SocketWatches.end()) {
			found->second->store(true);
			SocketWatches.erase(found);
		}
	}
}

inline int closesocket(SOCKET socket)
{
	OpenTSMacOS::CancelSocketWatch(socket);
	return(close(socket));
}
inline int WSAGetLastError(void) { return(errno); }
struct WSADATA { WORD wVersion; WORD wHighVersion; };
inline int WSAStartup(WORD version, WSADATA * data)
{
	if (data) { data->wVersion = version; data->wHighVersion = version; }
	return(0);
}
inline int WSACleanup(void) { return(0); }
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAEALREADY EALREADY
#define WSAEISCONN EISCONN
#define WSAEADDRINUSE EADDRINUSE
#define WSAECONNREFUSED ECONNREFUSED
#define WSAEINVAL EINVAL

constexpr long FD_READ = 1;
constexpr long FD_WRITE = 2;
#define WSAGETSELECTEVENT(value) LOWORD(value)
#define WSAGETSELECTERROR(value) HIWORD(value)
inline int WSAAsyncSelect(SOCKET socket, HWND window, UINT message, long events)
{
	int const flags = fcntl(socket, F_GETFL, 0);
	if (flags < 0 || fcntl(socket, F_SETFL, flags | O_NONBLOCK) != 0) return(SOCKET_ERROR);
	OpenTSMacOS::CancelSocketWatch(socket);
	if ((events & FD_READ) != 0 && window != nullptr && message != 0) {
		auto cancelled = std::make_shared<std::atomic_bool>(false);
		{
			std::lock_guard lock(OpenTSMacOS::SocketWatchMutex);
			OpenTSMacOS::SocketWatches[socket] = cancelled;
		}
		std::thread([socket, window, message, cancelled]() {
			while (!cancelled->load()) {
				fd_set read_set;
				FD_ZERO(&read_set);
				FD_SET(socket, &read_set);
				timeval timeout = {0, 10000};
				int const ready = select(socket + 1, &read_set, nullptr, nullptr, &timeout);
				if (ready < 0 && errno != EINTR) break;
				if (ready > 0 && FD_ISSET(socket, &read_set)) {
					PostMessage(window, message, 0, static_cast<LPARAM>(FD_READ));
				}
			}
		}).detach();
	}
	if ((events & FD_WRITE) != 0 && window != nullptr && message != 0) {
		PostMessage(window, message, 0, static_cast<LPARAM>(FD_WRITE));
	}
	return(0);
}
inline int WSACancelAsyncRequest(HANDLE) { return(0); }

using errno_t = int;
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif
#define _strdup strdup
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _access access
#define _unlink unlink
#define _mkdir(path) mkdir(path, 0700)
#define _fileno fileno
#define _O_BINARY 0
#define O_BINARY 0
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR

#ifndef strupr
inline char * strupr(char * text) { for (char * p = text; *p; p++) *p = static_cast<char>(std::toupper(*p)); return(text); }
#endif
#ifndef _strupr
inline char * _strupr(char * text) { return(strupr(text)); }
#endif
inline char * _strlwr(char * text) { for (char * p = text; *p; p++) *p = static_cast<char>(std::tolower(*p)); return(text); }
#ifndef strrev
inline char * strrev(char * text) { std::reverse(text, text + std::strlen(text)); return(text); }
#endif

inline int MultiByteToWideChar(UINT, DWORD, char const * source, int source_length,
	wchar_t * destination, int destination_length)
{
	if (!source) return(0);
	int const required = source_length < 0 ? static_cast<int>(strlen(source)) + 1 : source_length;
	if (!destination || destination_length == 0) return(required);
	int const copied = std::min(required, destination_length);
	for (int index = 0; index < copied; ++index) {
		destination[index] = static_cast<unsigned char>(source[index]);
	}
	if (source_length < 0) destination[copied - 1] = L'\0';
	return(copied == required ? copied : 0);
}

inline int WideCharToMultiByte(UINT, DWORD, wchar_t const * source, int source_length,
	char * destination, int destination_length, char const *, BOOL *)
{
	if (!source) return(0);
	int const required = source_length < 0 ? static_cast<int>(wcslen(source)) + 1 : source_length;
	if (!destination || destination_length == 0) return(required);
	int const copied = std::min(required, destination_length);
	for (int index = 0; index < copied; ++index) {
		destination[index] = source[index] <= 0xFF ? static_cast<char>(source[index]) : '?';
	}
	if (source_length < 0) destination[copied - 1] = '\0';
	return(copied == required ? copied : 0);
}

inline BOOL CharToOemBuff(char const * source, char * destination, DWORD length)
{
	if (!source || !destination) return(FALSE);
	std::string input(source, source + length);
	char * input_pointer = input.data();
	size_t input_left = input.size();
	char * output_pointer = destination;
	size_t output_left = length;
	iconv_t conversion = iconv_open("CP437", "WINDOWS-1252");
	if (conversion == reinterpret_cast<iconv_t>(-1)) {
		memmove(destination, source, length);
		return(FALSE);
	}
	bool const succeeded = iconv(conversion, &input_pointer, &input_left, &output_pointer, &output_left) != static_cast<size_t>(-1);
	iconv_close(conversion);
	if (!succeeded) memmove(destination, source, length);
	return(succeeded ? TRUE : FALSE);
}

inline HRESULT CLSIDFromString(wchar_t const * text, CLSID * identifier)
{
	if (!text || !identifier) return(E_INVALIDARG);
	unsigned data[11] = {};
	int const fields = swscanf(text, L"{%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x}",
		&data[0], &data[1], &data[2], &data[3], &data[4], &data[5], &data[6],
		&data[7], &data[8], &data[9], &data[10]);
	if (fields != 11) return(E_INVALIDARG);
	identifier->Data1 = data[0];
	identifier->Data2 = static_cast<std::uint16_t>(data[1]);
	identifier->Data3 = static_cast<std::uint16_t>(data[2]);
	for (int index = 0; index < 8; ++index) identifier->Data4[index] = static_cast<std::uint8_t>(data[index + 3]);
	return(S_OK);
}

inline HRESULT StringFromCLSID(REFCLSID identifier, LPOLESTR * text)
{
	if (!text) return(E_POINTER);
	*text = static_cast<wchar_t *>(malloc(39 * sizeof(wchar_t)));
	if (!*text) return(E_OUTOFMEMORY);
	swprintf(*text, 39, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		identifier.Data1, identifier.Data2, identifier.Data3,
		identifier.Data4[0], identifier.Data4[1], identifier.Data4[2], identifier.Data4[3],
		identifier.Data4[4], identifier.Data4[5], identifier.Data4[6], identifier.Data4[7]);
	return(S_OK);
}

inline void CoTaskMemFree(void * memory) { free(memory); }
inline void SysFreeString(wchar_t * string) { free(string); }
inline void * LocalFree(void * memory) { free(memory); return(nullptr); }

inline wchar_t const * GetCommandLineW()
{
	static std::wstring command_line;
	if (command_line.empty()) {
		int const count = *_NSGetArgc();
		char const * const * arguments = *_NSGetArgv();
		for (int index = 0; index < count; ++index) {
			if (index != 0) command_line += L' ';
			command_line += L'"';
			for (char const * byte = arguments[index]; *byte; ++byte) command_line += static_cast<unsigned char>(*byte);
			command_line += L'"';
		}
	}
	return(command_line.c_str());
}

inline LPWSTR * CommandLineToArgvW(wchar_t const *, int * count)
{
	int const argc = *_NSGetArgc();
	char const * const * arguments = *_NSGetArgv();
	size_t characters = 0;
	for (int index = 0; index < argc; ++index) characters += strlen(arguments[index]) + 1;
	size_t const pointers_size = static_cast<size_t>(argc + 1) * sizeof(LPWSTR);
	auto * allocation = static_cast<unsigned char *>(malloc(pointers_size + characters * sizeof(wchar_t)));
	if (!allocation) return(nullptr);
	auto ** result = reinterpret_cast<LPWSTR *>(allocation);
	auto * storage = reinterpret_cast<wchar_t *>(allocation + pointers_size);
	for (int index = 0; index < argc; ++index) {
		result[index] = storage;
		for (char const * byte = arguments[index];; ++byte, ++storage) {
			*storage = static_cast<unsigned char>(*byte);
			if (*byte == '\0') { ++storage; break; }
		}
	}
	result[argc] = nullptr;
	if (count) *count = argc;
	return(result);
}

inline HRESULT OleInitialize(void *) { return(S_OK); }
inline void OleUninitialize() {}
inline HWND FindWindow(char const *, char const *) { return(nullptr); }
inline BOOL SetForegroundWindow(HWND) { return(TRUE); }

inline void _makepath(char * path, char const *, char const * directory, char const * filename, char const * extension)
{
	char const * dot = extension && *extension && *extension != '.' ? "." : "";
	std::snprintf(path, MAX_PATH, "%s%s%s%s", directory ? directory : "", filename ? filename : "",
		dot, extension && *extension ? extension : "");
}

inline void _splitpath(char const * path, char * drive, char * directory, char * filename, char * extension)
{
	if (drive) drive[0] = '\0';
	// Windows _splitpath accepts both separators, and the game builds paths with backslashes.
	char const * slash = std::strrchr(path, '/');
	char const * backslash = std::strrchr(path, '\\');
	if (backslash && (!slash || backslash > slash)) slash = backslash;
	char const * base = slash ? slash + 1 : path;
	char const * dot = std::strrchr(base, '.');
	if (directory) { std::size_t length = slash ? static_cast<std::size_t>(slash - path + 1) : 0; std::memcpy(directory, path, length); directory[length] = '\0'; }
	if (filename) { std::size_t length = dot ? static_cast<std::size_t>(dot - base) : std::strlen(base); std::memcpy(filename, base, length); filename[length] = '\0'; }
	if (extension) std::strcpy(extension, dot ? dot : "");
}

inline void OutputDebugString(char const * text) { std::fputs(text, stderr); }
inline void OutputDebugStringA(char const * text) { OutputDebugString(text); }

LONG_PTR SetWindowLongPtr(HWND window, int index, LONG_PTR value);
LONG_PTR GetWindowLongPtr(HWND window, int index);
LONG SetWindowLong(HWND window, int index, LONG value);
LONG GetWindowLong(HWND window, int index);
BOOL ShowWindow(HWND window, int command);
BOOL UpdateWindow(HWND window);
BOOL CloseWindow(HWND window);
HWND SetCapture(HWND window);
HWND GetCapture(void);
BOOL ReleaseCapture(void);

#define SetWindowLongPtrA SetWindowLongPtr
#define GetWindowLongPtrA GetWindowLongPtr
#define SetWindowLongA SetWindowLong
#define GetWindowLongA GetWindowLong

BOOL PeekMessage(MSG * message, HWND window, UINT minimum, UINT maximum, UINT remove);
BOOL GetMessage(MSG * message, HWND window, UINT minimum, UINT maximum);
BOOL PostMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
void PostQuitMessage(int exit_code);
BOOL TranslateMessage(MSG const * message);
LRESULT DispatchMessage(MSG const * message);
LRESULT SendMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT DefWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CallWindowProc(WNDPROC procedure, HWND window, UINT message, WPARAM wparam, LPARAM lparam);

ATOM RegisterClass(WNDCLASS const * window_class);
BOOL UnregisterClass(LPCSTR class_name, HINSTANCE instance);
HWND CreateWindowEx(DWORD extended_style, LPCSTR class_name, LPCSTR title, DWORD style,
	int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance, LPVOID parameter);
BOOL DestroyWindow(HWND window);
HWND CreateDialogIndirectParam(HINSTANCE instance, LPCDLGTEMPLATE dialog_template, HWND parent,
	DLGPROC procedure, LPARAM parameter);
HWND CreateDialogParam(HINSTANCE instance, LPCSTR template_name, HWND parent, DLGPROC procedure, LPARAM parameter);
INT_PTR DialogBoxParam(HINSTANCE instance, LPCSTR template_name, HWND parent, DLGPROC procedure, LPARAM parameter);
BOOL EndDialog(HWND dialog, INT_PTR result);
BOOL EnumChildWindows(HWND parent, WNDENUMPROC procedure, LPARAM parameter);
HWND GetDlgItem(HWND dialog, int identifier);
HWND GetNextDlgTabItem(HWND dialog, HWND control, BOOL previous);
BOOL SetDlgItemText(HWND dialog, int identifier, LPCSTR text);
UINT GetDlgItemText(HWND dialog, int identifier, LPSTR text, int size);
LRESULT SendDlgItemMessage(HWND dialog, int identifier, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CheckDlgButton(HWND dialog, int identifier, UINT check);
UINT IsDlgButtonChecked(HWND dialog, int identifier);
BOOL EnableWindow(HWND window, BOOL enable);
BOOL IsWindowEnabled(HWND window);
BOOL IsWindowVisible(HWND window);
BOOL IsWindow(HWND window);
BOOL IsChild(HWND parent, HWND window);
HWND GetParent(HWND window);
HWND GetWindow(HWND window, UINT command);
HWND GetTopWindow(HWND parent);
int GetClassName(HWND window, LPSTR class_name, int size);
BOOL SetWindowText(HWND window, LPCSTR text);
HWND GetFocus(void);
BOOL SetForegroundWindow(HWND window);
BOOL GetWindowRect(HWND window, RECT * rectangle);
BOOL MoveWindow(HWND window, int x, int y, int width, int height, BOOL repaint);
BOOL SetWindowPos(HWND window, HWND insert_after, int x, int y, int width, int height, UINT flags);
BOOL SetRect(RECT * rectangle, int left, int top, int right, int bottom);
BOOL PtInRect(RECT const * rectangle, POINT point);
int MapWindowPoints(HWND source, HWND destination, POINT * points, UINT count);
HWND ChildWindowFromPoint(HWND parent, POINT point);
HMONITOR MonitorFromWindow(HWND window, DWORD flags);
BOOL GetMonitorInfo(HMONITOR monitor, MONITORINFO * information);
UINT_PTR SetTimer(HWND window, UINT_PTR identifier, UINT interval, TIMERPROC procedure);
BOOL KillTimer(HWND window, UINT_PTR identifier);
BOOL AdjustWindowRectEx(RECT * rectangle, DWORD style, BOOL menu, DWORD extended_style);
BOOL RedrawWindow(HWND window, RECT const * rectangle, HANDLE update_region, UINT flags);
BOOL BringWindowToTop(HWND window);
int GetDlgCtrlID(HWND window);

bool OpenTSMacOS_Is_Control_Window(HWND window);
LRESULT OpenTSMacOS_Send_Control_Message(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT OpenTSMacOS_Def_Control_Message(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL OpenTSMacOS_Get_Control_Rect(HWND window, RECT * rectangle, BOOL client);
BOOL OpenTSMacOS_Control_Point_Transform(HWND window, POINT * point, BOOL to_screen);
BOOL OpenTSMacOS_Invalidate_Control(HWND window);
HWND OpenTSMacOS_Set_Control_Focus(HWND window);
int OpenTSMacOS_Get_Control_Text(HWND window, LPSTR text, int size);
BOOL OpenTSMacOS_Get_Native_Window_Rect(HWND window, RECT * rectangle);
BOOL OpenTSMacOS_Move_Native_Window(HWND window, int x, int y, int width, int height, BOOL repaint);

BOOL GetClientRect(HWND window, RECT * rectangle);
BOOL GetUpdateRect(HWND window, RECT * rectangle, BOOL erase);
BOOL ClientToScreen(HWND window, POINT * point);
BOOL ScreenToClient(HWND window, POINT * point);
BOOL GetCursorPos(POINT * point);
BOOL SetCursorPos(int x, int y);
BOOL ClipCursor(RECT const * rectangle);
int ShowCursor(BOOL show);
int GetSystemMetrics(int index);
BOOL InvalidateRect(HWND window, RECT const * rectangle, BOOL erase);
BOOL ValidateRect(HWND window, RECT const * rectangle);
HWND SetActiveWindow(HWND window);
HWND SetFocus(HWND window);
HMENU GetMenu(HWND window);
BOOL IsDialogMessage(HWND dialog, MSG * message);
int TranslateAccelerator(HWND window, HACCEL accelerator, MSG * message);
int GetWindowText(HWND window, char * text, int size);
int GetWindowTextLength(HWND window);
int GetBkMode(HDC context);
COLORREF GetBkColor(HDC context);
COLORREF GetTextColor(HDC context);
COLORREF SetBkColor(HDC context, COLORREF color);
BOOL IntersectRect(RECT * destination, RECT const * left, RECT const * right);
HWND WindowFromPoint(POINT point);

inline HBRUSH CreateSolidBrush(COLORREF color) { return(reinterpret_cast<HBRUSH>(static_cast<UINT_PTR>(color) + 1)); }
inline HGDIOBJ GetStockObject(int object) { return(reinterpret_cast<HGDIOBJ>(static_cast<INT_PTR>(object) + 1)); }
inline HDC GetDC(HWND window) { return(window ? reinterpret_cast<HDC>(window) : reinterpret_cast<HDC>(1)); }
inline int ReleaseDC(HWND, HDC) { return(1); }
inline HGDIOBJ SelectObject(HDC, HGDIOBJ object) { return(object); }
inline BOOL DeleteObject(HGDIOBJ) { return(TRUE); }
inline int SetBkMode(HDC, int mode) { return(mode); }
inline COLORREF SetTextColor(HDC, COLORREF color) { return(color); }
inline BOOL GetTextExtentPoint32(HDC, LPCSTR text, int length, SIZE * size)
{
	if (!size) return(FALSE);
	size->cx = std::max(0, length) * 8;
	size->cy = 16;
	return(text != nullptr);
}
inline int DrawText(HDC, LPCSTR text, int length, RECT *, UINT) { return(text ? (length < 0 ? static_cast<int>(strlen(text)) : length) : 0); }
inline int SaveDC(HDC) { return(1); }
inline BOOL RestoreDC(HDC, int) { return(TRUE); }
inline int SetGraphicsMode(HDC, int mode) { return(mode); }
inline BOOL ModifyWorldTransform(HDC, void const *, DWORD) { return(TRUE); }
inline BOOL SetViewportOrgEx(HDC, int, int, POINT *) { return(TRUE); }
inline BOOL SetWindowOrgEx(HDC, int, int, POINT *) { return(TRUE); }
inline BOOL DPtoLP(HDC, POINT *, int) { return(TRUE); }
inline HFONT CreateFontIndirect(LOGFONT const *) { return(reinterpret_cast<HFONT>(1)); }
inline BOOL GetTextMetrics(HDC, TEXTMETRIC * metrics)
{
	if (!metrics) return(FALSE);
	std::memset(metrics, 0, sizeof(*metrics));
	metrics->tmHeight = 16;
	metrics->tmAveCharWidth = 8;
	metrics->tmMaxCharWidth = 8;
	return(TRUE);
}
inline HCURSOR LoadCursor(HINSTANCE, LPCSTR name) { return(reinterpret_cast<HCURSOR>(const_cast<char *>(name))); }
inline HCURSOR SetCursor(HCURSOR cursor) { return(cursor); }
inline DWORD GetWindowContextHelpId(HWND) { return(0); }
inline BOOL WinHelp(HWND, LPCSTR, UINT, ULONG_PTR) { return(FALSE); }
inline BOOL GetVersionEx(OSVERSIONINFO * information)
{
	if (!information) return(FALSE);
	information->dwMajorVersion = 0;
	information->dwMinorVersion = 0;
	information->dwBuildNumber = 0;
	information->dwPlatformId = VER_PLATFORM_WIN32_NT;
	information->szCSDVersion[0] = '\0';
	return(TRUE);
}
inline LONG RegOpenKeyEx(HKEY, LPCSTR, DWORD, DWORD, HKEY *) { return(1); }
inline LONG RegQueryValueEx(HKEY, LPCSTR, DWORD *, DWORD *, BYTE *, DWORD *) { return(1); }
inline LONG RegCloseKey(HKEY) { return(ERROR_SUCCESS); }

#define PostMessageA PostMessage
#define SendMessageA SendMessage
#define SetWindowTextA SetWindowText
int MessageBox(HWND owner, LPCSTR text, LPCSTR caption, UINT style);
int MessageBoxIndirect(MSGBOXPARAMS const * parameters);

SHORT GetKeyState(int key);
SHORT GetAsyncKeyState(int key);
UINT MapVirtualKey(UINT code, UINT map_type);
int ToAscii(UINT key, UINT scan_code, BYTE const * state, WORD * result, UINT flags);
int GetKeyNameText(LONG lparam, LPSTR buffer, int size);

inline BOOL TextOut(HDC, int, int, LPCSTR, int) { return(TRUE); }

#define ZeroMemory(pointer, size) std::memset((pointer), 0, (size))
#define wsprintf std::sprintf

HMODULE LoadLibrary(LPCTSTR path);
BOOL FreeLibrary(HMODULE module);
FARPROC GetProcAddress(HMODULE module, LPCSTR name);
int LoadString(HINSTANCE module, UINT identifier, LPSTR buffer, int size);
HRSRC FindResource(HMODULE module, LPCSTR name, LPCSTR type);
HGLOBAL LoadResource(HMODULE module, HRSRC resource);
LPVOID LockResource(HGLOBAL resource);
DWORD GetModuleFileName(HMODULE module, LPSTR path, DWORD size);
void const * OpenTSMacOS_Find_Dialog_Template(HMODULE preferred, LPCSTR name);
char OpenTSMacOS_To_CP1252(unsigned short character);

#define LoadLibraryA LoadLibrary
#define LoadStringA LoadString
#define FindResourceA FindResource
#define GetModuleFileNameA GetModuleFileName

namespace OpenTSMacOS
{
	struct FindState { std::vector<std::string> Paths; std::size_t Index = 0; };

	inline void FillFindData(std::string const & path, WIN32_FIND_DATAA * data)
	{
		if (!data) return;
		*data = {};
		struct stat status = {};
		if (stat(path.c_str(), &status) == 0) {
			if (S_ISDIR(status.st_mode)) data->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			data->ftCreationTime = OpenTSMacOSFileTime(status.st_birthtimespec.tv_sec, status.st_birthtimespec.tv_nsec, 0);
			data->ftLastAccessTime = OpenTSMacOSFileTime(status.st_atimespec.tv_sec, status.st_atimespec.tv_nsec, 0);
			data->ftLastWriteTime = OpenTSMacOSFileTime(status.st_mtimespec.tv_sec, status.st_mtimespec.tv_nsec, 0);
			std::uint64_t const size = static_cast<std::uint64_t>(status.st_size);
			data->nFileSizeLow = static_cast<DWORD>(size);
			data->nFileSizeHigh = static_cast<DWORD>(size >> 32);
		}
		auto const slash = path.find_last_of('/');
		std::string const name = slash == std::string::npos ? path : path.substr(slash + 1);
		std::snprintf(data->cFileName, sizeof(data->cFileName), "%s", name.c_str());
	}
}

inline HANDLE FindFirstFile(LPCSTR pattern, WIN32_FIND_DATAA * data)
{
	glob_t matches = {};
	std::string const native_pattern = OpenTSMacOS::NativePath(pattern);
	if (glob(native_pattern.c_str(), 0, nullptr, &matches) != 0 || matches.gl_pathc == 0) {
		globfree(&matches);
		return(INVALID_HANDLE_VALUE);
	}
	auto * state = new OpenTSMacOS::FindState;
	for (std::size_t index = 0; index < matches.gl_pathc; ++index) state->Paths.emplace_back(matches.gl_pathv[index]);
	globfree(&matches);
	OpenTSMacOS::FillFindData(state->Paths[0], data);
	return(state);
}

inline BOOL FindNextFile(HANDLE handle, WIN32_FIND_DATAA * data)
{
	if (!handle || handle == INVALID_HANDLE_VALUE) return(FALSE);
	auto * state = static_cast<OpenTSMacOS::FindState *>(handle);
	if (++state->Index >= state->Paths.size()) return(FALSE);
	OpenTSMacOS::FillFindData(state->Paths[state->Index], data);
	return(TRUE);
}

inline BOOL FindClose(HANDLE handle)
{
	if (!handle || handle == INVALID_HANDLE_VALUE) return(FALSE);
	delete static_cast<OpenTSMacOS::FindState *>(handle);
	return(TRUE);
}

constexpr DWORD STREAM_SEEK_SET = SEEK_SET;
constexpr DWORD STREAM_SEEK_CUR = SEEK_CUR;
constexpr DWORD STREAM_SEEK_END = SEEK_END;
constexpr DWORD STGM_READ = 0;
constexpr DWORD STGM_WRITE = 1;
constexpr DWORD STGM_READWRITE = 2;
constexpr DWORD STGM_CREATE = 0x1000;
constexpr DWORD CLSCTX_ALL = 0;
constexpr DWORD CLSCTX_INPROC_SERVER = 1;
constexpr DWORD CLSCTX_INPROC_HANDLER = 2;
constexpr DWORD CLSCTX_LOCAL_SERVER = 4;
constexpr DWORD REGCLS_MULTIPLEUSE = 1;

namespace OpenTSMacOS
{
	struct ClassRegistration
	{
		DWORD Identifier;
		GUID ClassIdentifier;
		IClassFactory * Factory;
	};

	inline std::mutex ClassRegistryMutex;
	inline std::vector<ClassRegistration> ClassRegistry;
	inline std::atomic<DWORD> NextClassRegistration {1};
}

inline HRESULT CoRegisterClassObject(REFCLSID class_id, IUnknown * factory, DWORD, DWORD, DWORD * registration)
{
	if (!factory || !registration) return(E_POINTER);
	IClassFactory * class_factory = nullptr;
	HRESULT const result = factory->QueryInterface(IID_IClassFactory, reinterpret_cast<void **>(&class_factory));
	if (FAILED(result)) return(result);

	DWORD const identifier = OpenTSMacOS::NextClassRegistration.fetch_add(1);
	std::lock_guard lock(OpenTSMacOS::ClassRegistryMutex);
	OpenTSMacOS::ClassRegistry.push_back({identifier, class_id, class_factory});
	*registration = identifier;
	return(S_OK);
}

inline HRESULT CoRevokeClassObject(DWORD registration)
{
	std::lock_guard lock(OpenTSMacOS::ClassRegistryMutex);
	auto const entry = std::find_if(OpenTSMacOS::ClassRegistry.begin(), OpenTSMacOS::ClassRegistry.end(),
		[registration](auto const & candidate) { return(candidate.Identifier == registration); });
	if (entry == OpenTSMacOS::ClassRegistry.end()) return(E_FAIL);
	entry->Factory->Release();
	OpenTSMacOS::ClassRegistry.erase(entry);
	return(S_OK);
}

inline HRESULT CoCreateInstance(REFCLSID class_id, IUnknown * outer, DWORD, REFIID interface_id, void ** object)
{
	if (!object) return(E_POINTER);
	*object = nullptr;
	std::lock_guard lock(OpenTSMacOS::ClassRegistryMutex);
	for (auto const & entry : OpenTSMacOS::ClassRegistry) {
		if (entry.ClassIdentifier == class_id) return(entry.Factory->CreateInstance(outer, interface_id, object));
	}
	return(E_FAIL);
}
