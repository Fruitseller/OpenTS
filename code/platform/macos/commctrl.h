#pragma once
#include "wincompat.h"

#define TRACKBAR_CLASS "msctls_trackbar32"
#define PROGRESS_CLASS "msctls_progress32"
#define WC_TREEVIEW "SysTreeView32"
#define WC_LISTVIEW "SysListView32"
#define WC_TABCONTROL "SysTabControl32"
#define WC_COMBOBOX "ComboBox"
#define HOTKEY_CLASS "msctls_hotkey32"

#define PBM_SETRANGE (WM_USER + 1)
#define PBM_SETPOS (WM_USER + 2)
#define TBM_GETPOS (WM_USER)
#define TBM_GETRANGEMIN (WM_USER + 1)
#define TBM_GETRANGEMAX (WM_USER + 2)
#define TBM_SETPOS (0x0400 + 5)
#define TBM_SETRANGE (0x0400 + 6)
#define TB_THUMBTRACK 5
#define HKM_SETHOTKEY (WM_USER + 1)
#define HKM_GETHOTKEY (WM_USER + 2)
#define TCM_FIRST 0x1300
#define TCM_SETITEMSIZE (TCM_FIRST + 41)

constexpr UINT TCIF_TEXT = 0x0001;
constexpr UINT TVIF_TEXT = 0x0001;
constexpr UINT TVIF_IMAGE = 0x0002;
constexpr UINT TVIF_HANDLE = 0x0010;
constexpr UINT TVIF_STATE = 0x0008;
constexpr UINT TVIF_SELECTEDIMAGE = 0x0020;
constexpr UINT TVIS_EXPANDED = 0x0020;
constexpr UINT TVE_COLLAPSE = 0x0001;
constexpr UINT TVE_EXPAND = 0x0002;

struct NMHDR { HWND hwndFrom; UINT_PTR idFrom; UINT code; };
struct TVITEMA {
	UINT mask;
	HTREEITEM hItem;
	UINT state;
	UINT stateMask;
	LPSTR pszText;
	int cchTextMax;
	int iImage;
	int iSelectedImage;
	int cChildren;
	LPARAM lParam;
};
using TVITEM = TVITEMA;
struct NMTREEVIEWA { NMHDR hdr; UINT action; TVITEMA itemOld; TVITEMA itemNew; POINT ptDrag; };
using NMTREEVIEW = NMTREEVIEWA;
using LPNMTREEVIEW = NMTREEVIEW *;
struct NMTVDISPINFOA { NMHDR hdr; TVITEMA item; };
using NMTVDISPINFO = NMTVDISPINFOA;
struct TCITEMA {
	UINT mask;
	DWORD dwState;
	DWORD dwStateMask;
	LPSTR pszText;
	int cchTextMax;
	int iImage;
	LPARAM lParam;
};
using TC_ITEM = TCITEMA;

inline BOOL TreeView_SelectItem(HWND, HTREEITEM) { return(TRUE); }
inline HIMAGELIST TreeView_CreateDragImage(HWND, HTREEITEM) { return(reinterpret_cast<HIMAGELIST>(1)); }
inline BOOL TreeView_GetItemRect(HWND, HTREEITEM, RECT * rectangle, BOOL)
{
	if (rectangle) *rectangle = {0, 0, 100, 16};
	return(rectangle != nullptr);
}
inline UINT TreeView_GetIndent(HWND) { return(16); }
inline BOOL TreeView_SelectDropTarget(HWND, HTREEITEM) { return(TRUE); }
inline BOOL TreeView_SelectSetFirstVisible(HWND, HTREEITEM) { return(TRUE); }
inline BOOL TreeView_Expand(HWND, HTREEITEM, UINT) { return(TRUE); }
inline HTREEITEM TreeView_GetRoot(HWND) { return(nullptr); }
inline HTREEITEM TreeView_GetFirstVisible(HWND) { return(nullptr); }
inline HTREEITEM TreeView_GetNextVisible(HWND, HTREEITEM) { return(nullptr); }
inline HTREEITEM TreeView_GetPrevVisible(HWND, HTREEITEM) { return(nullptr); }
inline HTREEITEM TreeView_GetNextSibling(HWND, HTREEITEM) { return(nullptr); }
inline HWND TreeView_GetEditControl(HWND) { return(nullptr); }
inline BOOL TreeView_GetItem(HWND, TVITEMA *) { return(FALSE); }
inline BOOL TreeView_SetItem(HWND, TVITEMA const *) { return(FALSE); }
inline int ListView_GetColumnWidth(HWND, int) { return(100); }
inline BOOL ListView_SetColumnWidth(HWND, int, int) { return(TRUE); }

inline int TabCtrl_GetItemCount(HWND) { return(0); }
inline int TabCtrl_GetCurSel(HWND) { return(0); }
inline BOOL TabCtrl_GetItemRect(HWND, int, RECT * rectangle)
{
	if (rectangle) *rectangle = {0, 0, 89, 16};
	return(rectangle != nullptr);
}
inline BOOL TabCtrl_GetItem(HWND, int, TCITEMA *) { return(FALSE); }

inline BOOL ImageList_BeginDrag(HIMAGELIST, int, int, int) { return(TRUE); }
inline BOOL ImageList_DragEnter(HWND, int, int) { return(TRUE); }
inline BOOL ImageList_DragMove(int, int) { return(TRUE); }
inline BOOL ImageList_DragShowNolock(BOOL) { return(TRUE); }
inline void ImageList_EndDrag(void) {}
inline BOOL ImageList_Destroy(HIMAGELIST) { return(TRUE); }

#define SNDMSG SendMessage
