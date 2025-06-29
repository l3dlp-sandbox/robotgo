// Copyright 2016 The go-vgo Project Developers. See the COPYRIGHT
// file at the top-level directory of this distribution and at
// https://github.com/go-vgo/robotgo/blob/master/LICENSE
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0>
//
// This file may not be copied, modified, or distributed
// except according to those terms.

// #if defined(USE_X11)
// 	#include <X11/Xresource.h>
// #endif

Bounds get_client(uintptr pid, int8_t isPid);

Bounds get_bounds(uintptr pid, int8_t isPid){
	// Check if the window is valid
	Bounds bounds;
	if (!is_valid()) { return bounds; }

    #if defined(IS_MACOSX)
		// Bounds bounds;
		AXValueRef axp = NULL;
		AXValueRef axs = NULL;
		AXUIElementRef AxID = AXUIElementCreateApplication(pid);
		AXUIElementRef AxWin = NULL;

		// Get the window from the application
		if (AXUIElementCopyAttributeValue(AxID, kAXFocusedWindowAttribute, (CFTypeRef *)&AxWin)
			!= kAXErrorSuccess || AxWin == NULL) {
			// If no focused window, try to get the main window
			if (AXUIElementCopyAttributeValue(AxID, kAXMainWindowAttribute, (CFTypeRef *)&AxWin)
				!= kAXErrorSuccess || AxWin == NULL) {
				goto exit;
			}
		}

		// Determine the current point of the window
		if (AXUIElementCopyAttributeValue(AxWin, kAXPositionAttribute, (CFTypeRef*) &axp)
			!= kAXErrorSuccess || axp == NULL) {
			goto exit;
		}

		// Determine the current size of the window
		if (AXUIElementCopyAttributeValue(AxWin, kAXSizeAttribute, (CFTypeRef*) &axs)
			!= kAXErrorSuccess || axs == NULL) {
			goto exit;
		}

		CGPoint p; CGSize s;
		// Attempt to convert both values into atomic types
		if (AXValueGetValue(axp, kAXValueCGPointType, &p) &&
			AXValueGetValue(axs, kAXValueCGSizeType, &s)) {
			bounds.X = p.x;
			bounds.Y = p.y;
			bounds.W = s.width;
			bounds.H = s.height;
		}

	exit:
		if (axp != NULL) { CFRelease(axp); }
		if (axs != NULL) { CFRelease(axs); }
		if (AxWin != NULL) { CFRelease(AxWin); }
		if (AxID != NULL) { CFRelease(AxID); }

		return bounds;
    #elif defined(USE_X11)
        // Ignore X errors
        XDismissErrors();
        MData win;
        win.XWin = (Window)pid;

        Bounds client = get_client(pid, isPid);
        Bounds frame = GetFrame(win);

        bounds.X = client.X - frame.X;
        bounds.Y = client.Y - frame.Y;
        bounds.W = client.W + frame.W;
        bounds.H = client.H + frame.H;

        return bounds;
    #elif defined(IS_WINDOWS)
        HWND hwnd = getHwnd(pid, isPid);

        RECT rect = { 0 };
        GetWindowRect(hwnd, &rect);

        bounds.X = rect.left;
        bounds.Y = rect.top;
        bounds.W = rect.right - rect.left;
        bounds.H = rect.bottom - rect.top;

        return bounds;
    #endif
}

Bounds get_client(uintptr pid, int8_t isPid) {
	// Check if the window is valid
	Bounds bounds;
	if (!is_valid()) { return bounds; }

	#if defined(IS_MACOSX)
		return get_bounds(pid, isPid);
	#elif defined(USE_X11)
        Display *rDisplay = XOpenDisplay(NULL);

		// Ignore X errors
		XDismissErrors();
		MData win;
        win.XWin = (Window)pid;

		// Property variables
		Window root, parent;
		Window* children;
		unsigned int count;
		int32_t x = 0, y = 0;

		// Check if the window is the root
		XQueryTree(rDisplay, win.XWin, &root, &parent, &children, &count);
		if (children) { XFree(children); }

		// Retrieve window attributes
		XWindowAttributes attr = { 0 };
		XGetWindowAttributes(rDisplay, win.XWin, &attr);

		// Coordinates must be translated
		if (parent != attr.root) {
			XTranslateCoordinates(rDisplay, win.XWin, attr.root, attr.x, attr.y, &x, &y, &parent);
		} else {
			x = attr.x;
			y = attr.y;
		}

		// Return resulting window bounds
		bounds.X = x;
		bounds.Y = y;
		bounds.W = attr.width;
		bounds.H = attr.height;
		XCloseDisplay(rDisplay);

		return bounds;
	#elif defined(IS_WINDOWS)
		HWND hwnd = getHwnd(pid, isPid);

		RECT rect = { 0 };
		GetClientRect(hwnd, &rect);

		POINT point;
		point.x = rect.left;
		point.y = rect.top;

		// Convert the client point to screen
		ClientToScreen(hwnd, &point);

		bounds.X = point.x;
		bounds.Y = point.y;
		bounds.W = rect.right - rect.left;
		bounds.H = rect.bottom - rect.top;

		return bounds;
	#endif
}
