/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "wincursor.h"

#include "_convert.h"
#include "_xmouse.h"
#include "convert.h"
#include "globals.h"
#include "goptions.h"
#include "shapeset.h"
#include "video.h"
#include "win.h"
#include "xmouse.h"
#ifdef OPENTS_MACOS
#include "platform/macos/macoswindow.h"
#endif

#include <cstring>
#include <vector>


struct CursorCacheEntry
{
	ShapeSet const * Shape;
	int Frame;
	int HotX;
	int HotY;
	HCURSOR Cursor;
};

// One cursor per shape frame the game actually asks for. MOUSE.SHP holds a few hundred
// of them, so the cache is emptied rather than grown when it fills.
static CursorCacheEntry _CursorCache[384];
static int _CursorCacheCount = 0;
static int _CacheScale = 0;

static ShapeSet const * _CurrentShape = NULL;
static int _CurrentFrame = 0;
static int _CurrentHotX = 0;
static int _CurrentHotY = 0;
static HCURSOR _CurrentCursor = NULL;
static bool _CursorVisible = true;


static void Destroy_Native_Cursor(HCURSOR cursor)
{
	if (cursor == NULL) {
		return;
	}
#ifdef OPENTS_MACOS
	OpenTSMacOS_Destroy_Cursor(cursor);
#else
	DestroyCursor(cursor);
#endif
}


static void Select_Native_Cursor(HCURSOR cursor, bool visible)
{
#ifdef OPENTS_MACOS
	OpenTSMacOS_Select_Cursor(cursor, visible);
#else
	SetCursor(visible ? cursor : NULL);
#endif
}


/// <summary>
/// Works out how much larger than its shape the cursor should be drawn.
/// </summary>
/// <returns>int; A whole multiple between one and eight.</returns>
static int Cursor_Scale(void)
{
	if (Options.CursorScale < 0) {
		return(1);
	}

	if (Options.CursorScale > 0) {
		return(Options.CursorScale > 8 ? 8 : Options.CursorScale);
	}

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	float smaller = scale.ScaleX < scale.ScaleY ? scale.ScaleX : scale.ScaleY;

	int result = (int)(smaller + 0.5f);
	if (result < 1) result = 1;
	if (result > 8) result = 8;
	return(result);
}


/// <summary>
/// Draws one shape frame into a Windows cursor.
/// The canvas covers the shape's whole frame rather than the trimmed part that holds
/// pixels, so the hotspot, which is measured from the frame's corner, still lands in the
/// right place. Palette entry zero is the transparent one.
/// </summary>
/// <returns>The cursor, or NULL if it could not be built.</returns>
static HCURSOR Build_Cursor(ShapeSet const * shape, int frame, int hotx, int hoty, int scale)
{
	if (shape == NULL || MouseDrawer == NULL) {
		return(NULL);
	}

	Rect rect = shape->Get_Rect(frame);
	unsigned char const * data = (unsigned char const *)shape->Get_Data(frame);

	if (!rect.Is_Valid() || data == NULL) {
		return(NULL);
	}

	int width = shape->Get_Width() * scale;
	int height = shape->Get_Height() * scale;

	if (width <= 0 || height <= 0) {
		return(NULL);
	}

#ifdef OPENTS_MACOS
	std::vector<unsigned char> pixels(static_cast<std::size_t>(width)
		* static_cast<std::size_t>(height) * 4);
#else
	BITMAPINFO info;
	memset(&info, '\0', sizeof(info));
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = -height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;

	void * bits = NULL;
	HBITMAP color = CreateDIBSection(NULL, &info, DIB_RGB_COLORS, &bits, NULL, 0);

	if (color == NULL) {
		return(NULL);
	}

	memset(bits, '\0', width * height * 4);
#endif

	// The shapes are palette indices and the primary is 565, so the drawer's table is
	// what turns one into the other.
	unsigned short const * table = (unsigned short const *)MouseDrawer->Get_Translate_Table();

	for (int y = 0; y < rect.Height; y++) {
		for (int x = 0; x < rect.Width; x++) {

			unsigned char index = data[y * rect.Width + x];
			if (index == 0) {
				continue;
			}

			unsigned short pixel = table[index];
			unsigned int red = ((pixel >> 11) & 0x1F) << 3;
			unsigned int green = ((pixel >> 5) & 0x3F) << 2;
			unsigned int blue = (pixel & 0x1F) << 3;

			for (int suby = 0; suby < scale; suby++) {
#ifdef OPENTS_MACOS
				unsigned char * row = pixels.data()
					+ (static_cast<std::size_t>((rect.Y + y) * scale + suby) * width
						+ static_cast<std::size_t>((rect.X + x) * scale)) * 4;
				for (int subx = 0; subx < scale; subx++) {
					row[subx * 4] = static_cast<unsigned char>(red);
					row[subx * 4 + 1] = static_cast<unsigned char>(green);
					row[subx * 4 + 2] = static_cast<unsigned char>(blue);
					row[subx * 4 + 3] = 255;
				}
#else
				unsigned int * row = (unsigned int *)bits
					+ ((rect.Y + y) * scale + suby) * width + (rect.X + x) * scale;
				unsigned int argb = 0xFF000000U | (red << 16) | (green << 8) | blue;
				for (int subx = 0; subx < scale; subx++) {
					row[subx] = argb;
				}
#endif
			}
		}
	}

	int cursor_hotx = hotx * scale;
	int cursor_hoty = hoty * scale;
	if (cursor_hotx < 0) cursor_hotx = 0;
	if (cursor_hoty < 0) cursor_hoty = 0;
	if (cursor_hotx >= width) cursor_hotx = width - 1;
	if (cursor_hoty >= height) cursor_hoty = height - 1;

#ifdef OPENTS_MACOS
	return(OpenTSMacOS_Create_Cursor(pixels.data(), width, height, cursor_hotx, cursor_hoty));
#else
	// A color cursor carries its transparency in the alpha channel, but Windows still
	// wants a mask bitmap alongside it.
	int mask_pitch = ((width + 15) / 16) * 2;
	char * mask_bits = new char[mask_pitch * height];
	memset(mask_bits, '\0', mask_pitch * height);
	HBITMAP mask = CreateBitmap(width, height, 1, 1, mask_bits);
	delete [] mask_bits;

	ICONINFO icon;
	icon.fIcon = FALSE;
	icon.xHotspot = cursor_hotx;
	icon.yHotspot = cursor_hoty;
	icon.hbmMask = mask;
	icon.hbmColor = color;
	HCURSOR cursor = (HCURSOR)CreateIconIndirect(&icon);

	DeleteObject(mask);
	DeleteObject(color);
	return(cursor);
#endif
}


static void Flush_Cursor_Cache(void)
{
	for (int index = 0; index < _CursorCacheCount; index++) {
		if (_CursorCache[index].Cursor != NULL) {
			Destroy_Native_Cursor(_CursorCache[index].Cursor);
		}
	}

	_CursorCacheCount = 0;
	_CurrentCursor = NULL;
}


/// <summary>
/// Selects the cursor for a shape frame, building it if it has not been seen before.
/// </summary>
/// <param name="shape">The shape set the frame belongs to.</param>
/// <param name="frame">Which frame of it to show.</param>
/// <param name="hotx">The point within the frame that does the pointing.</param>
/// <param name="hoty">The same, vertically.</param>
/// <param name="apply">Should the cursor be shown straight away?</param>
void Win_Cursor_Set(ShapeSet const * shape, int frame, int hotx, int hoty, bool apply)
{
	int scale = Cursor_Scale();

	if (scale != _CacheScale) {
		Flush_Cursor_Cache();
		_CacheScale = scale;
	}

	_CurrentShape = shape;
	_CurrentFrame = frame;
	_CurrentHotX = hotx;
	_CurrentHotY = hoty;

	HCURSOR cursor = NULL;

	for (int index = 0; index < _CursorCacheCount; index++) {
		CursorCacheEntry & entry = _CursorCache[index];
		if (entry.Shape == shape && entry.Frame == frame) {
			if (entry.HotX != hotx || entry.HotY != hoty) {
				if (entry.Cursor != NULL) {
					Destroy_Native_Cursor(entry.Cursor);
				}
				entry.Cursor = Build_Cursor(shape, frame, hotx, hoty, scale);
				entry.HotX = hotx;
				entry.HotY = hoty;
			}
			cursor = entry.Cursor;
			break;
		}
	}

	if (cursor == NULL) {
		if (_CursorCacheCount >= (int)(sizeof(_CursorCache) / sizeof(_CursorCache[0]))) {
			Flush_Cursor_Cache();
		}

		cursor = Build_Cursor(shape, frame, hotx, hoty, scale);

		if (cursor != NULL) {
			CursorCacheEntry & entry = _CursorCache[_CursorCacheCount++];
			entry.Shape = shape;
			entry.Frame = frame;
			entry.HotX = hotx;
			entry.HotY = hoty;
			entry.Cursor = cursor;
		}
	}

	_CurrentCursor = cursor;

	if (apply) {
		Select_Native_Cursor(_CurrentCursor, _CursorVisible);
	}
}


/// <summary>
/// Shows or hides the pointer.
/// </summary>
void Win_Cursor_Set_Visible(bool visible)
{
	_CursorVisible = visible;

	if (MouseCursor != NULL && MouseCursor->Is_Captured()) {
		Select_Native_Cursor(_CurrentCursor, visible);
	}
}


/// <summary>
/// Puts the game's pointer back after Windows has asked what the cursor should be.
/// </summary>
/// <returns>bool; Was the cursor the game's to choose? While a dialog has the mouse it
/// is not, and Windows keeps its own arrow.</returns>
bool Win_Cursor_Handle_Set_Cursor(void)
{
	if (MouseCursor == NULL || !MouseCursor->Is_Captured()) {
		return(false);
	}

	Select_Native_Cursor(_CurrentCursor, _CursorVisible);
	return(true);
}


/// <summary>
/// Rebuilds the pointer if the window has been resized enough to want a different size.
/// </summary>
void Win_Cursor_Refresh(void)
{
	if (_CurrentShape != NULL && Cursor_Scale() != _CacheScale) {
		Win_Cursor_Set(_CurrentShape, _CurrentFrame, _CurrentHotX, _CurrentHotY,
			MouseCursor != NULL && MouseCursor->Is_Captured());
	}
}


/// <summary>
/// Releases every cursor the game built.
/// </summary>
void Win_Cursor_Shutdown(void)
{
	Select_Native_Cursor(NULL, false);
	Flush_Cursor_Cache();
	_CurrentShape = NULL;
}
