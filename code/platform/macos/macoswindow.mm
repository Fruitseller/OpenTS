/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#define BOOL CocoaBOOL
#include <AppKit/AppKit.h>
#include <CoreGraphics/CoreGraphics.h>
#undef BOOL
#undef FALSE
#undef TRUE

#include "macoswindow.h"
#undef interface

#include <array>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <set>
#include <string>

extern LRESULT CALLBACK Windows_Procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
extern int CALLBACK WinMain(HINSTANCE instance, HINSTANCE previous_instance, char * command_line, int command_show);

namespace
{
	constexpr UINT MessageDestroy = 0x0002;
	constexpr UINT MessageCreate = 0x0001;
	constexpr UINT MessageMove = 0x0003;
	constexpr UINT MessageSize = 0x0005;
	constexpr UINT MessageActivateApplication = 0x001C;
	constexpr UINT MessageClose = 0x0010;
	constexpr UINT MessagePaint = 0x000F;
	constexpr UINT MessageDisplayChange = 0x007E;
	constexpr UINT MessageQuit = 0x0012;
	constexpr UINT MessageKeyDown = 0x0100;
	constexpr UINT MessageKeyUp = 0x0101;
	constexpr UINT MessageSystemKeyDown = 0x0104;
	constexpr UINT MessageSystemKeyUp = 0x0105;
	constexpr UINT MessageMouseMove = 0x0200;
	constexpr UINT MessageLeftButtonDown = 0x0201;
	constexpr UINT MessageLeftButtonUp = 0x0202;
	constexpr UINT MessageLeftButtonDoubleClick = 0x0203;
	constexpr UINT MessageRightButtonDown = 0x0204;
	constexpr UINT MessageRightButtonUp = 0x0205;
	constexpr UINT MessageRightButtonDoubleClick = 0x0206;
	constexpr UINT MessageMiddleButtonDown = 0x0207;
	constexpr UINT MessageMiddleButtonUp = 0x0208;
	constexpr UINT MessageMiddleButtonDoubleClick = 0x0209;
	constexpr UINT MessageMouseWheel = 0x020A;

	constexpr WPARAM SizeMinimized = 1;
	constexpr int ShowNormal = 1;

	std::deque<MSG> MessageQueue;
	std::mutex MessageMutex;
	std::array<bool, 256> KeyState = {};
	NSWindow * MainNativeWindow = nil;
	NSCursor * HiddenCursor = nil;
	NSCursor * SelectedCursor = nil;
	int CursorDisplayCount = 0;
	bool SelectedCursorVisible = false;
	bool UseSystemCursor = true;
	bool HasClipRectangle = false;
	RECT ClipRectangle = {};
	bool ClosingWindow = false;
	bool InitialActivationDone = false;
	bool QuitRequested = false;

	NSCursor * Get_Hidden_Cursor(void)
	{
		if (HiddenCursor == nil) {
			HiddenCursor = [[NSCursor alloc]
				initWithImage:[[NSImage alloc] initWithSize:NSMakeSize(1.0, 1.0)]
				hotSpot:NSZeroPoint];
		}
		return(HiddenCursor);
	}

	void Apply_Selected_Cursor(void)
	{
		if (CursorDisplayCount < 0 || (!UseSystemCursor
			&& (!SelectedCursorVisible || SelectedCursor == nil))) {
			[Get_Hidden_Cursor() set];
		} else if (UseSystemCursor) {
			[[NSCursor arrowCursor] set];
		} else {
			[SelectedCursor set];
		}
	}

	LPARAM Pack_Coordinates(int x, int y)
	{
		return(static_cast<LPARAM>(static_cast<WORD>(x))
			| (static_cast<LPARAM>(static_cast<WORD>(y)) << 16));
	}

	void Queue_Message(HWND window, UINT message, WPARAM wparam = 0, LPARAM lparam = 0)
	{
		MSG entry = {};
		entry.hwnd = window;
		entry.message = message;
		entry.wParam = wparam;
		entry.lParam = lparam;
		entry.time = static_cast<DWORD>([NSProcessInfo processInfo].systemUptime * 1000.0);

		std::lock_guard lock(MessageMutex);
		MessageQueue.push_back(entry);
	}

	unsigned short Virtual_Key_For_Event(NSEvent * event)
	{
		switch (event.keyCode) {
			case 36: return(0x0D); // Return
			case 48: return(0x09); // Tab
			case 51: return(0x08); // Backspace
			case 53: return(0x1B); // Escape
			case 71: return(0x0C); // Keypad clear
			case 76: return(0x0D); // Keypad enter
			case 114: return(0x2D); // Help/Insert
			case 115: return(0x24); // Home
			case 116: return(0x21); // Page up
			case 117: return(0x2E); // Forward delete
			case 119: return(0x23); // End
			case 121: return(0x22); // Page down
			case 123: return(0x25); // Left
			case 124: return(0x27); // Right
			case 125: return(0x28); // Down
			case 126: return(0x26); // Up
			case 122: return(0x70);
			case 120: return(0x71);
			case 99: return(0x72);
			case 118: return(0x73);
			case 96: return(0x74);
			case 97: return(0x75);
			case 98: return(0x76);
			case 100: return(0x77);
			case 101: return(0x78);
			case 109: return(0x79);
			case 103: return(0x7A);
			case 111: return(0x7B);
			default: break;
		}

		NSString * characters = event.charactersIgnoringModifiers;
		if (characters.length == 0) {
			return(0);
		}

		unichar character = [characters characterAtIndex:0];
		if (character >= 'a' && character <= 'z') {
			return(static_cast<unsigned short>(character - 'a' + 'A'));
		}
		if ((character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9')) {
			return(static_cast<unsigned short>(character));
		}

		switch (character) {
			case ' ': return(0x20);
			case ';': return(0xBA);
			case '=': return(0xBB);
			case ',': return(0xBC);
			case '-': return(0xBD);
			case '.': return(0xBE);
			case '/': return(0xBF);
			case '`': return(0xC0);
			case '[': return(0xDB);
			case '\\': return(0xDC);
			case ']': return(0xDD);
			case '\'': return(0xDE);
			default: return(0);
		}
	}

	unsigned short Modifier_Key_For_Event(NSEvent * event)
	{
		switch (event.keyCode) {
			case 56:
			case 60: return(0x10); // Shift
			case 59:
			case 62: return(0x11); // Control
			case 58:
			case 61: return(0x12); // Option/Alt
			case 55: return(0x5B); // Left Command
			case 54: return(0x5C); // Right Command
			default: return(0);
		}
	}

	NSEventModifierFlags Modifier_Flag_For_Key(unsigned short key)
	{
		switch (key) {
			case 0x10: return(NSEventModifierFlagShift);
			case 0x11: return(NSEventModifierFlagControl);
			case 0x12: return(NSEventModifierFlagOption);
			case 0x5B:
			case 0x5C: return(NSEventModifierFlagCommand);
			default: return(0);
		}
	}

	bool Is_Quit_Key_Event(NSEvent * event)
	{
		if (event.type != NSEventTypeKeyDown) {
			return(false);
		}

		NSEventModifierFlags constexpr shortcut_modifiers = NSEventModifierFlagCommand
			| NSEventModifierFlagControl | NSEventModifierFlagOption | NSEventModifierFlagShift;
		if ((event.modifierFlags & shortcut_modifiers) != NSEventModifierFlagCommand) {
			return(false);
		}

		NSString * characters = event.charactersIgnoringModifiers;
		return(characters.length == 1
			&& [characters caseInsensitiveCompare:@"q"] == NSOrderedSame);
	}

	POINT Client_Point_For_Event(NSView * view, NSEvent * event)
	{
		NSPoint point = [view convertPoint:event.locationInWindow fromView:nil];
		NSRect bounds = view.bounds;
		return(POINT{static_cast<LONG>(point.x), static_cast<LONG>(bounds.size.height - point.y)});
	}

	POINT Screen_Point_For_Event(NSEvent * event)
	{
		NSPoint point = event.locationInWindow;
		point = [event.window convertPointToScreen:point];
		NSScreen * screen = NSScreen.mainScreen;
		return(POINT{static_cast<LONG>(point.x), static_cast<LONG>(NSMaxY(screen.frame) - point.y)});
	}

	void Queue_Key_Event(NSEvent * event, bool released)
	{
		unsigned short const key = Virtual_Key_For_Event(event);
		if (key == 0) {
			return;
		}

		bool const was_down = KeyState[key];
		KeyState[key] = !released;
		LPARAM const lparam = was_down ? static_cast<LPARAM>(1u << 30) : 0;
		bool const system_key = (event.modifierFlags & NSEventModifierFlagOption) != 0;
		HWND const focus = GetFocus();
		HWND const target = OpenTSMacOS_Is_Control_Window(focus) ? focus : (__bridge HWND)event.window;
		Queue_Message(target,
			system_key ? (released ? MessageSystemKeyUp : MessageSystemKeyDown)
				: (released ? MessageKeyUp : MessageKeyDown), key, lparam);
		if (!released && OpenTSMacOS_Is_Control_Window(focus) && !system_key
			&& !(event.modifierFlags & (NSEventModifierFlagControl | NSEventModifierFlagCommand))) {
			NSString * characters = event.characters;
			for (NSUInteger index = 0; index < characters.length; ++index) {
				unichar character = [characters characterAtIndex:index];
				if (character == NSDeleteCharacter) character = VK_BACK;
				if (character < 0xF700 || character > 0xF8FF) {
					Queue_Message(target, WM_CHAR, static_cast<unsigned char>(OpenTSMacOS_To_CP1252(character)), lparam);
				}
			}
		}
	}

	void Queue_Mouse_Event(NSView * view, NSEvent * event, UINT message)
	{
		POINT const point = Client_Point_For_Event(view, event);
		WPARAM wparam = 0;
		NSUInteger const pressed = [NSEvent pressedMouseButtons];
		if ((pressed & (1 << 0)) != 0 || message == MessageLeftButtonDown || message == MessageLeftButtonDoubleClick) {
			wparam |= MK_LBUTTON;
		}
		if ((pressed & (1 << 1)) != 0 || message == MessageRightButtonDown || message == MessageRightButtonDoubleClick) {
			wparam |= MK_RBUTTON;
		}
		if ((pressed & (1 << 2)) != 0 || message == MessageMiddleButtonDown || message == MessageMiddleButtonDoubleClick) {
			wparam |= MK_MBUTTON;
		}
		if ((event.modifierFlags & NSEventModifierFlagShift) != 0) {
			wparam |= MK_SHIFT;
		}
		if ((event.modifierFlags & NSEventModifierFlagControl) != 0) {
			wparam |= MK_CONTROL;
		}
		Queue_Message((__bridge HWND)event.window, message, wparam, Pack_Coordinates(point.x, point.y));
	}
}

@interface OpenTSContentView : NSView
@property(nonatomic, strong) NSTrackingArea * trackingArea;
@end

@implementation OpenTSContentView

- (CocoaBOOL)acceptsFirstResponder
{
	return(YES);
}

- (void)updateTrackingAreas
{
	[super updateTrackingAreas];
	if (self.trackingArea != nil) {
		[self removeTrackingArea:self.trackingArea];
	}
	self.trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
		options:NSTrackingMouseMoved | NSTrackingActiveAlways | NSTrackingInVisibleRect
		owner:self userInfo:nil];
	[self addTrackingArea:self.trackingArea];
}

- (void)resetCursorRects
{
	[super resetCursorRects];
	NSCursor * cursor = Get_Hidden_Cursor();
	if (CursorDisplayCount >= 0 && UseSystemCursor) {
		cursor = NSCursor.arrowCursor;
	} else if (CursorDisplayCount >= 0 && SelectedCursorVisible && SelectedCursor != nil) {
		cursor = SelectedCursor;
	}
	[self addCursorRect:self.bounds cursor:cursor];
}

- (void)keyDown:(NSEvent *)event { Queue_Key_Event(event, false); }
- (void)keyUp:(NSEvent *)event { Queue_Key_Event(event, true); }

- (void)flagsChanged:(NSEvent *)event
{
	unsigned short const key = Modifier_Key_For_Event(event);
	if (key == 0) {
		return;
	}

	bool const released = (event.modifierFlags & Modifier_Flag_For_Key(key)) == 0;
	bool const was_down = KeyState[key];
	KeyState[key] = !released;
	LPARAM const lparam = was_down ? static_cast<LPARAM>(1u << 30) : 0;
	Queue_Message((__bridge HWND)event.window, released ? MessageKeyUp : MessageKeyDown, key, lparam);
}

- (void)mouseMoved:(NSEvent *)event { Queue_Mouse_Event(self, event, MessageMouseMove); }
- (void)mouseDragged:(NSEvent *)event { Queue_Mouse_Event(self, event, MessageMouseMove); }
- (void)rightMouseDragged:(NSEvent *)event { Queue_Mouse_Event(self, event, MessageMouseMove); }
- (void)otherMouseDragged:(NSEvent *)event { Queue_Mouse_Event(self, event, MessageMouseMove); }

- (void)mouseDown:(NSEvent *)event
{
	Queue_Mouse_Event(self, event, event.clickCount > 1 ? MessageLeftButtonDoubleClick : MessageLeftButtonDown);
}

- (void)mouseUp:(NSEvent *)event { Queue_Mouse_Event(self, event, MessageLeftButtonUp); }

- (void)rightMouseDown:(NSEvent *)event
{
	Queue_Mouse_Event(self, event, event.clickCount > 1 ? MessageRightButtonDoubleClick : MessageRightButtonDown);
}

- (void)rightMouseUp:(NSEvent *)event { Queue_Mouse_Event(self, event, MessageRightButtonUp); }

- (void)otherMouseDown:(NSEvent *)event
{
	if (event.buttonNumber == 2) {
		Queue_Mouse_Event(self, event, event.clickCount > 1 ? MessageMiddleButtonDoubleClick : MessageMiddleButtonDown);
	}
}

- (void)otherMouseUp:(NSEvent *)event
{
	if (event.buttonNumber == 2) {
		Queue_Mouse_Event(self, event, MessageMiddleButtonUp);
	}
}

- (void)scrollWheel:(NSEvent *)event
{
	short const delta = static_cast<short>(event.scrollingDeltaY > 0.0 ? 120 : -120);
	POINT const point = Screen_Point_For_Event(event);
	WPARAM const wparam = static_cast<WPARAM>(static_cast<WORD>(delta)) << 16;
	Queue_Message((__bridge HWND)event.window, MessageMouseWheel, wparam, Pack_Coordinates(point.x, point.y));
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	Queue_Message((__bridge HWND)self.window, MessagePaint);
}

@end

static void OpenTSMacOS_Request_Quit(void)
{
	QuitRequested = true;
}

@interface OpenTSWindowDelegate : NSObject<NSWindowDelegate, NSApplicationDelegate>
@end

@implementation OpenTSWindowDelegate

- (void)windowDidResize:(NSNotification *)notification
{
	NSWindow * window = notification.object;
	NSRect const bounds = window.contentView.bounds;
	Queue_Message((__bridge HWND)window, MessageSize, 0,
		Pack_Coordinates(static_cast<int>(bounds.size.width), static_cast<int>(bounds.size.height)));
}

- (void)windowDidMove:(NSNotification *)notification
{
	Queue_Message((__bridge HWND)notification.object, MessageMove);
}

- (void)windowDidChangeBackingProperties:(NSNotification *)notification
{
	Queue_Message((__bridge HWND)notification.object, MessageDisplayChange);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
	Queue_Message((__bridge HWND)notification.object, MessageSize, SizeMinimized, 0);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
	InitialActivationDone = true;
	Queue_Message((__bridge HWND)notification.object, MessageActivateApplication, TRUE);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
	for (std::size_t key = 0; key < KeyState.size(); ++key) {
		if (KeyState[key]) {
			KeyState[key] = false;
			Queue_Message((__bridge HWND)notification.object, MessageKeyUp, key,
				static_cast<LPARAM>(1u << 30));
		}
	}
	Queue_Message((__bridge HWND)notification.object, MessageActivateApplication, FALSE);
}

- (CocoaBOOL)windowShouldClose:(NSWindow *)window
{
	if (ClosingWindow) {
		return(YES);
	}
	// The engine ignores WM_CLOSE, so defer the main-window exit until AppKit
	// has returned control to the event pump.
	if ((__bridge NSWindow *)MainNativeWindow == window) {
		OpenTSMacOS_Request_Quit();
	}
	Queue_Message((__bridge HWND)window, MessageClose);
	return(NO);
}

- (void)windowWillClose:(NSNotification *)notification
{
	Queue_Message((__bridge HWND)notification.object, MessageDestroy);
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	OpenTSMacOS_Request_Quit();
	return(NSTerminateCancel);
}

@end

static OpenTSWindowDelegate * WindowDelegate = nil;

// A borderless NSWindow refuses key and main status by default, which would
// leave the game waiting for its first focus message forever.
@interface OpenTSWindow : NSWindow
@end

@implementation OpenTSWindow
- (CocoaBOOL)canBecomeKeyWindow { return(YES); }
- (CocoaBOOL)canBecomeMainWindow { return(YES); }
@end

static void Configure_Window_Mode(NSWindow * window, bool windowed)
{
	if (windowed) {
		[window center];
	} else {
		window.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
		window.level = NSNormalWindowLevel;
	}
}

HWND OpenTSMacOS_Create_Window(int width, int height, bool windowed)
{
	@autoreleasepool {
		NSApplication * application = NSApplication.sharedApplication;
		[application setActivationPolicy:NSApplicationActivationPolicyRegular];
		[application finishLaunching];
		if (application.mainMenu == nil) {
			NSMenu * menu = [[NSMenu alloc] init];
			NSMenuItem * application_item = [[NSMenuItem alloc] init];
			[menu addItem:application_item];
			NSMenu * application_menu = [[NSMenu alloc] init];
			[application_menu addItemWithTitle:@"Quit OpenTS" action:@selector(terminate:)
				keyEquivalent:@"q"];
			application_item.submenu = application_menu;
			application.mainMenu = menu;
		}

		NSUInteger style = NSWindowStyleMaskBorderless;
		NSRect frame = NSScreen.mainScreen.frame;
		if (windowed) {
			style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
				| NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
			frame = NSMakeRect(0.0, 0.0, static_cast<CGFloat>(width), static_cast<CGFloat>(height));
		}

		MainNativeWindow = [[OpenTSWindow alloc] initWithContentRect:frame styleMask:style
			backing:NSBackingStoreBuffered defer:NO];
		MainNativeWindow.title = @"Tiberian Sun";
		MainNativeWindow.acceptsMouseMovedEvents = YES;
		MainNativeWindow.releasedWhenClosed = NO;
		MainNativeWindow.contentView = [[OpenTSContentView alloc] initWithFrame:frame];
		WindowDelegate = [[OpenTSWindowDelegate alloc] init];
		MainNativeWindow.delegate = WindowDelegate;
		application.delegate = WindowDelegate;

		Configure_Window_Mode(MainNativeWindow, windowed);

		Queue_Message((__bridge HWND)MainNativeWindow, MessageCreate);
		[MainNativeWindow makeKeyAndOrderFront:nil];
		[MainNativeWindow makeFirstResponder:MainNativeWindow.contentView];
		[application activateIgnoringOtherApps:YES];
		return((__bridge HWND)MainNativeWindow);
	}
}

void OpenTSMacOS_Destroy_Window(HWND window)
{
	@autoreleasepool {
		NSWindow * native_window = (__bridge NSWindow *)window;
		if (native_window == nil) {
			return;
		}
		ClosingWindow = true;
		[native_window close];
		ClosingWindow = false;
		if (native_window == MainNativeWindow) {
			MainNativeWindow = nil;
			WindowDelegate = nil;
		}
	}
}

void * OpenTSMacOS_Native_Window_Handle(HWND window)
{
	return(window);
}

HWND OpenTSMacOS_Get_Main_Native_Window(void)
{
	return((__bridge HWND)MainNativeWindow);
}

void OpenTSMacOS_Pump_Events(void)
{
	bool quit_requested = false;
	@autoreleasepool {
		NSApplication * application = NSApplication.sharedApplication;
		// Activation of an app launched without user interaction can be refused,
		// which would leave the game waiting for its first focus message forever.
		// Forcing it only once lets the user later switch away without the game
		// pulling focus back on the next pump.
		if (!InitialActivationDone && !application.active && MainNativeWindow != nil
			&& MainNativeWindow.visible && !MainNativeWindow.keyWindow) {
			[application activateIgnoringOtherApps:YES];
			[MainNativeWindow makeKeyAndOrderFront:nil];
		}
		for (;;) {
			NSEvent * event = [application nextEventMatchingMask:NSEventMaskAny
				untilDate:NSDate.distantPast inMode:NSDefaultRunLoopMode dequeue:YES];
			if (event == nil) {
				break;
			}
			// The manual event pump does not run the main menu's Cmd-Q key equivalent.
			if (Is_Quit_Key_Event(event)) {
				OpenTSMacOS_Request_Quit();
			} else {
				[application sendEvent:event];
			}
			if (QuitRequested) {
				break;
			}
		}
		[application updateWindows];
		quit_requested = QuitRequested;
	}
	if (quit_requested) {
		std::exit(EXIT_SUCCESS);
	}
}

BOOL OpenTSMacOS_Get_Client_Rect(HWND window, RECT * rectangle)
{
	if (rectangle == nullptr) {
		return(FALSE);
	}
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Get_Control_Rect(window, rectangle, TRUE));
	}
	NSWindow * native_window = (window != nullptr && window == (__bridge HWND)MainNativeWindow) ? MainNativeWindow : MainNativeWindow;
	if (native_window == nil) {
		return(FALSE);
	}
	NSRect const bounds = native_window.contentView.bounds;
	*rectangle = {0, 0, static_cast<LONG>(bounds.size.width), static_cast<LONG>(bounds.size.height)};
	return(TRUE);
}

BOOL OpenTSMacOS_Get_Window_Rect(HWND window, RECT * rectangle)
{
	if (rectangle == nullptr) {
		return(FALSE);
	}
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Get_Control_Rect(window, rectangle, FALSE));
	}
	NSWindow * native_window = (window != nullptr && window == (__bridge HWND)MainNativeWindow) ? MainNativeWindow : MainNativeWindow;
	if (native_window == nil) {
		return(FALSE);
	}
	NSRect const frame = native_window.frame;
	CGFloat const screen_top = NSMaxY(NSScreen.mainScreen.frame);
	*rectangle = {static_cast<LONG>(frame.origin.x),
		static_cast<LONG>(screen_top - NSMaxY(frame)),
		static_cast<LONG>(NSMaxX(frame)),
		static_cast<LONG>(screen_top - frame.origin.y)};
	return(TRUE);
}

BOOL OpenTSMacOS_Client_To_Screen(HWND window, POINT * point)
{
	if (point == nullptr) {
		return(FALSE);
	}
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Control_Point_Transform(window, point, TRUE));
	}
	NSWindow * native_window = (window != nullptr && window == (__bridge HWND)MainNativeWindow) ? MainNativeWindow : MainNativeWindow;
	if (native_window == nil) {
		return(FALSE);
	}
	NSView * view = native_window.contentView;
	NSPoint native_point = NSMakePoint(point->x, view.bounds.size.height - point->y);
	native_point = [view convertPoint:native_point toView:nil];
	native_point = [native_window convertPointToScreen:native_point];
	point->x = static_cast<LONG>(native_point.x);
	point->y = static_cast<LONG>(NSMaxY(NSScreen.mainScreen.frame) - native_point.y);
	return(TRUE);
}

BOOL OpenTSMacOS_Screen_To_Client(HWND window, POINT * point)
{
	if (point == nullptr) {
		return(FALSE);
	}
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Control_Point_Transform(window, point, FALSE));
	}
	NSWindow * native_window = (window != nullptr && window == (__bridge HWND)MainNativeWindow) ? MainNativeWindow : MainNativeWindow;
	if (native_window == nil) {
		return(FALSE);
	}
	NSView * view = native_window.contentView;
	NSPoint native_point = NSMakePoint(point->x, NSMaxY(NSScreen.mainScreen.frame) - point->y);
	native_point = [native_window convertPointFromScreen:native_point];
	native_point = [view convertPoint:native_point fromView:nil];
	point->x = static_cast<LONG>(native_point.x);
	point->y = static_cast<LONG>(view.bounds.size.height - native_point.y);
	return(TRUE);
}

BOOL OpenTSMacOS_Get_Cursor_Pos(POINT * point)
{
	if (point == nullptr) {
		return(FALSE);
	}
	NSPoint const native_point = NSEvent.mouseLocation;
	point->x = static_cast<LONG>(native_point.x);
	point->y = static_cast<LONG>(NSMaxY(NSScreen.mainScreen.frame) - native_point.y);
	if (HasClipRectangle) {
		point->x = std::clamp(point->x, ClipRectangle.left, ClipRectangle.right - 1);
		point->y = std::clamp(point->y, ClipRectangle.top, ClipRectangle.bottom - 1);
	}
	return(TRUE);
}

BOOL OpenTSMacOS_Set_Cursor_Pos(int x, int y)
{
	if (HasClipRectangle) {
		x = std::clamp<LONG>(x, ClipRectangle.left, ClipRectangle.right - 1);
		y = std::clamp<LONG>(y, ClipRectangle.top, ClipRectangle.bottom - 1);
	}
	return(CGWarpMouseCursorPosition(CGPointMake(x, y)) == kCGErrorSuccess);
}

BOOL OpenTSMacOS_Clip_Cursor(RECT const * rectangle)
{
	HasClipRectangle = rectangle != nullptr;
	if (rectangle != nullptr) {
		ClipRectangle = *rectangle;
		POINT point;
		if (OpenTSMacOS_Get_Cursor_Pos(&point)) {
			OpenTSMacOS_Set_Cursor_Pos(point.x, point.y);
		}
	}
	return(TRUE);
}

int OpenTSMacOS_Show_Cursor(BOOL show)
{
	CursorDisplayCount += show ? 1 : -1;
	if (show && CursorDisplayCount >= 0) {
		UseSystemCursor = true;
	}
	Apply_Selected_Cursor();
	if (MainNativeWindow != nil) {
		[MainNativeWindow invalidateCursorRectsForView:MainNativeWindow.contentView];
	}
	return(CursorDisplayCount);
}

HCURSOR OpenTSMacOS_Create_Cursor(void const * rgba_pixels, int width, int height,
	int hot_x, int hot_y)
{
	if (rgba_pixels == nullptr || width <= 0 || height <= 0) {
		return(nullptr);
	}

	@autoreleasepool {
		NSBitmapImageRep * bitmap = [[NSBitmapImageRep alloc]
			initWithBitmapDataPlanes:nil pixelsWide:width pixelsHigh:height bitsPerSample:8
			samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace
			bitmapFormat:NSBitmapFormatAlphaNonpremultiplied bytesPerRow:width * 4 bitsPerPixel:32];
		if (bitmap == nil || bitmap.bitmapData == nullptr) {
			return(nullptr);
		}

		memcpy(bitmap.bitmapData, rgba_pixels,
			static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
		NSImage * image = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
		[image addRepresentation:bitmap];
		NSCursor * cursor = [[NSCursor alloc] initWithImage:image
			hotSpot:NSMakePoint(hot_x, hot_y)];
		[bitmap release];
		[image release];
		return((__bridge HCURSOR)cursor);
	}
}

void OpenTSMacOS_Destroy_Cursor(HCURSOR cursor)
{
	if (cursor == nullptr) {
		return;
	}
	NSCursor * native_cursor = (__bridge NSCursor *)cursor;
	if (native_cursor == SelectedCursor) {
		SelectedCursor = nil;
		SelectedCursorVisible = false;
		UseSystemCursor = true;
		Apply_Selected_Cursor();
	}
	[native_cursor release];
}

void OpenTSMacOS_Select_Cursor(HCURSOR cursor, bool visible)
{
	SelectedCursor = cursor != nullptr ? (__bridge NSCursor *)cursor : nil;
	SelectedCursorVisible = visible;
	UseSystemCursor = false;
	Apply_Selected_Cursor();
	if (MainNativeWindow != nil) {
		[MainNativeWindow invalidateCursorRectsForView:MainNativeWindow.contentView];
	}
}

int OpenTSMacOS_Display_Refresh_Rate(HWND window)
{
	NSScreen * screen = window != nullptr ? [(__bridge NSWindow *)window screen] : NSScreen.mainScreen;
	return(screen != nil ? static_cast<int>(screen.maximumFramesPerSecond) : 0);
}

int * OpenTSMacOS_Enumerate_Display_Modes(HWND window, int min_width, int min_height,
	int max_width, int max_height)
{
	NSScreen * screen = window != nullptr ? [(__bridge NSWindow *)window screen] : NSScreen.mainScreen;
	NSNumber * screen_number = screen.deviceDescription[@"NSScreenNumber"];
	if (screen_number == nil) {
		return(nullptr);
	}

	CGDirectDisplayID display = static_cast<CGDirectDisplayID>(screen_number.unsignedIntValue);
	CFArrayRef display_modes = CGDisplayCopyAllDisplayModes(display, nullptr);
	if (display_modes == nullptr) {
		return(nullptr);
	}

	std::set<std::pair<int, int>> modes;
	CFIndex const count = CFArrayGetCount(display_modes);
	for (CFIndex index = 0; index < count; ++index) {
		CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(display_modes, index);
		int const width = static_cast<int>(CGDisplayModeGetWidth(mode));
		int const height = static_cast<int>(CGDisplayModeGetHeight(mode));
		if (width >= min_width && width <= max_width && height >= min_height && height <= max_height) {
			modes.emplace(width, height);
		}
	}
	CFRelease(display_modes);

	if (modes.empty()) {
		return(nullptr);
	}

	int * result = new int[(modes.size() + 1) * 2];
	std::size_t index = 0;
	for (auto const & mode : modes) {
		result[index * 2] = mode.first;
		result[index * 2 + 1] = mode.second;
		++index;
	}
	result[index * 2] = 0;
	result[index * 2 + 1] = 0;
	return(result);
}

BOOL PeekMessage(MSG * message, HWND window, UINT minimum, UINT maximum, UINT remove)
{
	OpenTSMacOS_Pump_Events();
	std::lock_guard lock(MessageMutex);
	for (auto entry = MessageQueue.begin(); entry != MessageQueue.end(); ++entry) {
		if ((window == nullptr || entry->hwnd == window)
		&& (minimum == 0 || entry->message >= minimum)
		&& (maximum == 0 || entry->message <= maximum)) {
			if (message != nullptr) {
				*message = *entry;
			}
			if (remove == PM_REMOVE) {
				MessageQueue.erase(entry);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

BOOL GetMessage(MSG * message, HWND window, UINT minimum, UINT maximum)
{
	if (message == nullptr) {
		return(FALSE);
	}

	std::lock_guard lock(MessageMutex);
	for (auto entry = MessageQueue.begin(); entry != MessageQueue.end(); ++entry) {
		if ((window == nullptr || entry->hwnd == window)
		&& (minimum == 0 || entry->message >= minimum)
		&& (maximum == 0 || entry->message <= maximum)) {
			*message = *entry;
			MessageQueue.erase(entry);
			return(message->message != MessageQuit);
		}
	}
	return(FALSE);
}

BOOL PostMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	Queue_Message(window, message, wparam, lparam);
	return(TRUE);
}

void PostQuitMessage(int exit_code)
{
	Queue_Message(nullptr, MessageQuit, static_cast<WPARAM>(exit_code));
}

BOOL TranslateMessage(MSG const *)
{
	return(TRUE);
}

LRESULT DispatchMessage(MSG const * message)
{
	if (message == nullptr) {
		return(0);
	}
	if (OpenTSMacOS_Is_Control_Window(message->hwnd)) {
		return(OpenTSMacOS_Dispatch_Control_Message(message->hwnd, message->message, message->wParam, message->lParam));
	}
	return(Windows_Procedure(message->hwnd, message->message, message->wParam, message->lParam));
}

LRESULT SendMessage(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Send_Control_Message(window, message, wparam, lparam));
	}
	return(window == (__bridge HWND)MainNativeWindow
		? Windows_Procedure(window, message, wparam, lparam) : 0);
}

LRESULT DefWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Def_Control_Message(window, message, wparam, lparam));
	}
	if (message == MessageClose) {
		OpenTSMacOS_Destroy_Window(window);
	}
	return(0);
}

SHORT GetKeyState(int key)
{
	return(key >= 0 && key < static_cast<int>(KeyState.size()) && KeyState[key] ? static_cast<SHORT>(0x8000) : 0);
}

SHORT GetAsyncKeyState(int key)
{
	return(GetKeyState(key));
}

UINT MapVirtualKey(UINT code, UINT)
{
	return(code);
}

// MapVirtualKey is the identity above, so the scan-code field carries the virtual-key code.
int GetKeyNameText(LONG lparam, LPSTR buffer, int size)
{
	if (buffer == nullptr || size <= 0) return(0);
	UINT const code = (static_cast<ULONG>(lparam) >> 16) & 0xFF;
	char single[2] = {0, 0};
	char const * name = nullptr;
	if ((code >= 'A' && code <= 'Z') || (code >= '0' && code <= '9')) {
		single[0] = static_cast<char>(code);
		name = single;
	} else {
		switch (code) {
			case 0x08: name = "Backspace"; break;
			case 0x09: name = "Tab"; break;
			case 0x0D: name = "Enter"; break;
			case 0x10: name = "Shift"; break;
			case 0x11: name = "Ctrl"; break;
			case 0x12: name = "Alt"; break;
			case 0x14: name = "Caps Lock"; break;
			case 0x1B: name = "Esc"; break;
			case 0x20: name = "Space"; break;
			case 0x21: name = "Page Up"; break;
			case 0x22: name = "Page Down"; break;
			case 0x23: name = "End"; break;
			case 0x24: name = "Home"; break;
			case 0x25: name = "Left"; break;
			case 0x26: name = "Up"; break;
			case 0x27: name = "Right"; break;
			case 0x28: name = "Down"; break;
			case 0x2D: name = "Insert"; break;
			case 0x2E: name = "Delete"; break;
			default: break;
		}
	}
	char numbered[4];
	if (name == nullptr && code >= 0x70 && code <= 0x7B) {
		std::snprintf(numbered, sizeof(numbered), "F%u", code - 0x70 + 1);
		name = numbered;
	}
	if (name == nullptr) {
		buffer[0] = '\0';
		return(0);
	}
	std::snprintf(buffer, static_cast<std::size_t>(size), "%s", name);
	return(static_cast<int>(std::strlen(buffer)));
}

int ToAscii(UINT key, UINT, BYTE const * state, WORD * result, UINT)
{
	if (result == nullptr) {
		return(0);
	}
	bool const shift = state != nullptr && (state[0x10] & 0x80) != 0;
	if (key >= 'A' && key <= 'Z') {
		*result = static_cast<WORD>(shift ? key : key - 'A' + 'a');
		return(1);
	}
	if (key >= '0' && key <= '9') {
		static constexpr char ShiftedDigits[] = ")!@#$%^&*(";
		*result = static_cast<WORD>(shift ? ShiftedDigits[key - '0'] : key);
		return(1);
	}
	struct Translation { UINT Key; char Plain; char Shifted; };
	static constexpr Translation Translations[] = {
		{0x20, ' ', ' '}, {0xBA, ';', ':'}, {0xBB, '=', '+'}, {0xBC, ',', '<'},
		{0xBD, '-', '_'}, {0xBE, '.', '>'}, {0xBF, '/', '?'}, {0xC0, '`', '~'},
		{0xDB, '[', '{'}, {0xDC, '\\', '|'}, {0xDD, ']', '}'}, {0xDE, '\'', '"'}
	};
	for (Translation const & translation : Translations) {
		if (translation.Key == key) {
			*result = static_cast<WORD>(shift ? translation.Shifted : translation.Plain);
			return(1);
		}
	}
	return(0);
}

BOOL GetClientRect(HWND window, RECT * rectangle)
{
	return(OpenTSMacOS_Is_Control_Window(window)
		? OpenTSMacOS_Get_Control_Rect(window, rectangle, TRUE)
		: OpenTSMacOS_Get_Client_Rect(window, rectangle));
}
BOOL ClientToScreen(HWND window, POINT * point)
{
	return(OpenTSMacOS_Is_Control_Window(window)
		? OpenTSMacOS_Control_Point_Transform(window, point, TRUE)
		: OpenTSMacOS_Client_To_Screen(window, point));
}
BOOL ScreenToClient(HWND window, POINT * point)
{
	return(OpenTSMacOS_Is_Control_Window(window)
		? OpenTSMacOS_Control_Point_Transform(window, point, FALSE)
		: OpenTSMacOS_Screen_To_Client(window, point));
}
BOOL GetCursorPos(POINT * point) { return(OpenTSMacOS_Get_Cursor_Pos(point)); }
BOOL SetCursorPos(int x, int y) { return(OpenTSMacOS_Set_Cursor_Pos(x, y)); }
BOOL ClipCursor(RECT const * rectangle) { return(OpenTSMacOS_Clip_Cursor(rectangle)); }
int ShowCursor(BOOL show) { return(OpenTSMacOS_Show_Cursor(show)); }

int GetSystemMetrics(int index)
{
	NSRect const screen = NSScreen.mainScreen.frame;
	switch (index) {
		case SM_CXSCREEN:
		case SM_CXFULLSCREEN: return(static_cast<int>(screen.size.width));
		case SM_CYSCREEN:
		case SM_CYFULLSCREEN: return(static_cast<int>(screen.size.height));
		case SM_CXBORDER:
		case SM_CYBORDER: return(1);
		case SM_CXDRAG:
		case SM_CYDRAG: return(4);
		case SM_SWAPBUTTON: return(0);
		default: return(0);
	}
}

BOOL InvalidateRect(HWND window, RECT const *, BOOL)
{
	if (window == nullptr) {
		return(FALSE);
	}
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Invalidate_Control(window));
	}
	[((__bridge NSWindow *)window).contentView setNeedsDisplay:YES];
	return(TRUE);
}

HWND SetActiveWindow(HWND window)
{
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Set_Control_Focus(window));
	}
	if (window != nullptr) {
		[(__bridge NSWindow *)window makeKeyAndOrderFront:nil];
	}
	return(window);
}

HWND SetFocus(HWND window)
{
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Set_Control_Focus(window));
	}
	if (window != nullptr) {
		NSWindow * native_window = (__bridge NSWindow *)window;
		[native_window makeFirstResponder:native_window.contentView];
	}
	return(OpenTSMacOS_Set_Control_Focus(window));
}

HMENU GetMenu(HWND) { return(nullptr); }
int TranslateAccelerator(HWND, HACCEL, MSG *) { return(0); }

int GetWindowText(HWND window, char * text, int size)
{
	if (text == nullptr || size <= 0) {
		return(0);
	}
	if (OpenTSMacOS_Is_Control_Window(window)) {
		return(OpenTSMacOS_Get_Control_Text(window, text, size));
	}
	NSString * title = window != nullptr ? [(__bridge NSWindow *)window title] : @"";
	std::string const utf8 = title.UTF8String != nullptr ? title.UTF8String : "";
	int const length = std::min(size - 1, static_cast<int>(utf8.size()));
	std::memcpy(text, utf8.data(), static_cast<std::size_t>(length));
	text[length] = '\0';
	return(length);
}

BOOL OpenTSMacOS_Get_Native_Window_Rect(HWND window, RECT * rectangle)
{
	if (window == nullptr || rectangle == nullptr) return(FALSE);
	NSRect const frame = [(__bridge NSWindow *)window frame];
	NSScreen * screen = NSScreen.mainScreen;
	rectangle->left = static_cast<LONG>(frame.origin.x);
	rectangle->top = static_cast<LONG>(NSMaxY(screen.frame) - NSMaxY(frame));
	rectangle->right = rectangle->left + static_cast<LONG>(frame.size.width);
	rectangle->bottom = rectangle->top + static_cast<LONG>(frame.size.height);
	return(TRUE);
}

BOOL OpenTSMacOS_Move_Native_Window(HWND window, int x, int y, int width, int height, BOOL repaint)
{
	if (window == nullptr) return(FALSE);
	NSScreen * screen = NSScreen.mainScreen;
	NSRect const frame = NSMakeRect(x, NSMaxY(screen.frame) - y - height, width, height);
	[(__bridge NSWindow *)window setFrame:frame display:repaint != FALSE];
	return(TRUE);
}

int MessageBox(HWND owner, LPCSTR text, LPCSTR caption, UINT style)
{
	@autoreleasepool {
		NSAlert * alert = [[NSAlert alloc] init];
		alert.messageText = caption != nullptr ? [NSString stringWithUTF8String:caption] : @"OpenTS";
		alert.informativeText = text != nullptr ? [NSString stringWithUTF8String:text] : @"";
		UINT const icon = style & 0xF0;
		if (icon == MB_ICONERROR) {
			alert.alertStyle = NSAlertStyleCritical;
		} else if (icon == MB_ICONWARNING || icon == MB_ICONQUESTION) {
			alert.alertStyle = NSAlertStyleWarning;
		} else {
			alert.alertStyle = NSAlertStyleInformational;
		}

		if ((style & MB_YESNO) == MB_YESNO) {
			[alert addButtonWithTitle:@"Yes"];
			[alert addButtonWithTitle:@"No"];
		} else {
			[alert addButtonWithTitle:@"OK"];
			if ((style & MB_OKCANCEL) == MB_OKCANCEL) {
				[alert addButtonWithTitle:@"Cancel"];
			}
		}

		NSModalResponse const response = [alert runModal];
		if ((style & MB_YESNO) == MB_YESNO) {
			return(response == NSAlertFirstButtonReturn ? IDYES : IDNO);
		}
		return(response == NSAlertFirstButtonReturn ? IDOK : IDCANCEL);
	}
}

int MessageBoxIndirect(MSGBOXPARAMS const * parameters)
{
	if (parameters == nullptr) {
		return(0);
	}
	return(MessageBox(parameters->hwndOwner, parameters->lpszText,
		parameters->lpszCaption, parameters->dwStyle));
}

#ifdef OPENTS_MACOS_WINDOW_TEST
bool OpenTSMacOS_Test_Text_Input(void)
{
	MessageQueue.clear();
	NSEvent * event = [NSEvent keyEventWithType:NSEventTypeKeyDown location:NSZeroPoint
		modifierFlags:0 timestamp:0 windowNumber:0 context:nil characters:@"a"
		charactersIgnoringModifiers:@"a" isARepeat:NO keyCode:0];
	Queue_Key_Event(event, false);
	return(MessageQueue.size() == 2 && MessageQueue[0].hwnd == GetFocus()
		&& MessageQueue[0].message == WM_KEYDOWN && MessageQueue[1].hwnd == GetFocus()
		&& MessageQueue[1].message == WM_CHAR && MessageQueue[1].wParam == 'a');
}

bool OpenTSMacOS_Test_Focus_Activation(void)
{
	InitialActivationDone = false;
	OpenTSWindowDelegate * delegate = [[OpenTSWindowDelegate alloc] init];
	NSNotification * notification = [NSNotification notificationWithName:@"OpenTSFocusTest"
		object:nil];
	[delegate windowDidBecomeKey:notification];
	return(InitialActivationDone);
}

bool OpenTSMacOS_Test_Fullscreen_Window_Level(void)
{
	NSRect const frame = NSMakeRect(0.0, 0.0, 32.0, 32.0);
	OpenTSWindow * window = [[OpenTSWindow alloc] initWithContentRect:frame
		styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:NO];
	Configure_Window_Mode(window, false);
	return(window.level == NSNormalWindowLevel);
}

bool OpenTSMacOS_Test_Request_Quit(void)
{
	QuitRequested = false;
	OpenTSWindowDelegate * delegate = [[OpenTSWindowDelegate alloc] init];
	NSApplicationTerminateReply const reply =
		[delegate applicationShouldTerminate:NSApplication.sharedApplication];
	return(reply == NSTerminateCancel && QuitRequested);
}

bool OpenTSMacOS_Test_Command_Q(void)
{
	QuitRequested = false;
	HWND const window = OpenTSMacOS_Create_Window(32, 32, true);
	NSWindow * native_window = (__bridge NSWindow *)window;
	NSEvent * event = [NSEvent keyEventWithType:NSEventTypeKeyDown location:NSZeroPoint
		modifierFlags:NSEventModifierFlagCommand timestamp:NSProcessInfo.processInfo.systemUptime
		windowNumber:native_window.windowNumber context:nil characters:@"q"
		charactersIgnoringModifiers:@"q" isARepeat:NO keyCode:12];
	[NSApplication.sharedApplication postEvent:event atStart:YES];
	OpenTSMacOS_Pump_Events();
	OpenTSMacOS_Destroy_Window(window);
	return(false);
}
#endif

#ifndef OPENTS_MACOS_WINDOW_TEST
int main(int, char **)
{
	@autoreleasepool {
		char command_line[] = "";
		return(WinMain(nullptr, nullptr, command_line, ShowNormal));
	}
}
#endif
