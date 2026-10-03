#ifndef STARS_NATIVE_H
#define STARS_NATIVE_H

// Helpers the Win32 build needs where Stars' Win16 types and behavior differ
// from Win32. See docs/NATIVE-PORT.md.

#include <direct.h>
#include <io.h>
#include <limits.h>
#include <stdlib.h>

/*
 * Win16 points
 *
 * Win16 POINT held 16-bit ints, and Stars writes records holding points to
 * its files. Stars' points are POINT16 to keep that layout; Win32 POINT holds
 * LONGs, so points convert where they pass into or out of the Win32 API.
 */

typedef struct tagPOINT16 {
    int16_t x;
    int16_t y;
} POINT16;

// PointFrom16 widens a Stars point to a Win32 point.
static inline POINT PointFrom16(POINT16 pt) {
    POINT out = {pt.x, pt.y};
    return out;
}

// PointTo16 narrows a Win32 point to a Stars point.
static inline POINT16 PointTo16(POINT pt) {
    POINT16 out = {(int16_t)pt.x, (int16_t)pt.y};
    return out;
}

// GetCursorPos16 stores the cursor position in a Stars point.
static inline BOOL GetCursorPos16(POINT16 *ppt) {
    POINT pt;
    BOOL  ret = GetCursorPos(&pt);
    *ppt = PointTo16(pt);
    return ret;
}

// ScreenToClient16 converts a Stars point from screen to client coordinates.
static inline BOOL ScreenToClient16(HWND hwnd, POINT16 *ppt) {
    POINT pt = PointFrom16(*ppt);
    BOOL  ret = ScreenToClient(hwnd, &pt);
    *ppt = PointTo16(pt);
    return ret;
}

// ClientToScreen16 converts a Stars point from client to screen coordinates.
static inline BOOL ClientToScreen16(HWND hwnd, POINT16 *ppt) {
    POINT pt = PointFrom16(*ppt);
    BOOL  ret = ClientToScreen(hwnd, &pt);
    *ppt = PointTo16(pt);
    return ret;
}

// MapWindowPoints16 maps c Stars points from one window's coordinates to
// another's.
static inline int MapWindowPoints16(HWND hwndFrom, HWND hwndTo, POINT16 *ppt, UINT c) {
    int ret = 0;
    for (UINT i = 0; i < c; i++) {
        POINT pt = PointFrom16(ppt[i]);
        ret = MapWindowPoints(hwndFrom, hwndTo, &pt, 1);
        ppt[i] = PointTo16(pt);
    }
    return ret;
}

// qsort16 sorts like Stars' Win16 CRT qsort, which decides the order of
// elements with equal keys. Turn generation depends on that order.
void qsort16(void *base, size_t count, size_t width, int (*compare)(const void *, const void *));

// GetTextExtent measures text like Win16 GetTextExtent, which Win32 dropped:
// the width in the low word and the height in the high word.
DWORD GetTextExtent(HDC hdc, LPCSTR str, int len);

// FrameWndProcDeferred is the frame window class's procedure; it wraps
// FrameWndProc (see native.c).
LRESULT CALLBACK FrameWndProcDeferred(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
