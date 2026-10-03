#ifndef STARS_DECOMPILED_WIN16DEFINES_H
#define STARS_DECOMPILED_WIN16DEFINES_H

#include <direct.h>
#include <io.h>
#include <limits.h>
#include <stdlib.h>

#define qsort qsort16

// SIGNHIWORD is the high word from signed 16-to-32 extension (the x86 CWD
// instruction), not the upper half of an already-wide value.
#define SIGNHIWORD(value) ((int16_t)(((uint16_t)(value) & 0x8000) ? -1 : 0))

// Win16 APIs whose Win32 equivalents changed signature
#define GetTextExtent GetTextExtent16
#undef CreateWindow
#define CreateWindow CreateWindow16

// shims

// qsortSwap16 exchanges complete elements, including native-width pointers.
static inline void qsortSwap16(unsigned char *a, unsigned char *b, size_t width) {
    while (width != 0) {
        --width;
        unsigned char value = a[width];
        a[width] = b[width];
        b[width] = value;
    }
}

// qsort16 preserves the ordering of equal keys in Stars' Win16 CRT qsort.
// Reconstructed from 0024:0866-09c6, matching qsort.asm in C700 and MSVC
// MLIBCEW.LIB. The initial sorted check, first-element pivot, and strict
// partition boundaries matter: ICompLong compares planet X coordinates only.
static inline void qsort16(void *base, size_t count, size_t width, int (*compare)(const void *, const void *)) {
    if (count < 2 || width == 0)
        return;

    unsigned char *items = (unsigned char *)base;
    size_t         i;
    for (i = 1; i < count; ++i) {
        if ((int16_t)compare(items + i * width, items + (i - 1) * width) < 0)
            break;
    }
    if (i == count)
        return;

    // Process the smaller partition first, bounding pending ranges by log2(count).
    size_t lows[sizeof(size_t) * CHAR_BIT], highs[sizeof(size_t) * CHAR_BIT];
    size_t pending = 0, lo = 0, hi = count - 1;
    for (;;) {
        if (lo < hi) {
            size_t left = lo, right = hi + 1;
            size_t lastLess = lo, firstGreater = hi;
            for (;;) {
                while (++left != hi) {
                    int order = (int16_t)compare(items + left * width, items + lo * width);
                    if (order > 0)
                        break;
                    if (order < 0)
                        lastLess = left;
                }
                for (;;) {
                    --right;
                    int order = (int16_t)compare(items + lo * width, items + right * width);
                    if (order > 0)
                        break;
                    if (order < 0)
                        firstGreater = right;
                    else if (right == lo)
                        break;
                }
                if (right <= left)
                    break;
                qsortSwap16(items + left * width, items + right * width, width);
                firstGreater = right;
                lastLess = left;
            }
            qsortSwap16(items + lo * width, items + right * width, width);

            if (hi - firstGreater >= lastLess - lo) {
                lows[pending] = firstGreater;
                highs[pending++] = hi;
                hi = lastLess;
            } else {
                lows[pending] = lo;
                highs[pending++] = lastLess;
                lo = firstGreater;
            }
        } else {
            if (pending == 0)
                return;
            lo = lows[--pending];
            hi = highs[pending];
        }
    }
}

static inline DWORD GetTextExtent16(HDC hdc, LPCSTR str, int len) {
    SIZE size;

    if (!GetTextExtentPoint32A(hdc, str, len, &size))
        return 0;

    return MAKELONG((WORD)size.cx, (WORD)size.cy);
}

/*
 * Win16 window creation
 *
 * Win16 CW_USEDEFAULT is the 16-bit 0x8000, which Stars stores in its window
 * rectangles as -32768 when Stars.ini holds no position. Win32 reads that as
 * a real coordinate and places the window far off screen, so it is mapped to
 * the native CW_USEDEFAULT.
 */

// CreateWindow16 creates a window, mapping Win16 CW_USEDEFAULT positions and
// sizes to the native value.
static inline HWND CreateWindow16(LPCSTR cls, LPCSTR name, DWORD style, int x, int y, int cx, int cy, HWND parent, HMENU menu, HINSTANCE inst, LPVOID param) {
    if (x == -32768 || x == 0x8000)
        x = CW_USEDEFAULT;
    if (cx == -32768 || cx == 0x8000)
        cx = CW_USEDEFAULT;
    return CreateWindowExA(0, cls, name, style, x, y, cx, cy, parent, menu, inst, param);
}

/*
 * Win16 frame restore
 *
 * FrameWndProc handles SC_RESTORE and SC_MAXIMIZE before DefWindowProc
 * restores the window, and while a submitted turn waits it asks, in a task
 * modal MessageBox, whether to load the new turn or unsubmit. Win16 delivered
 * those commands from the message loop. Wine's macOS driver sends SC_RESTORE
 * from inside its handler for a Dock un-minimize and takes no more input
 * events until that send returns, so the MessageBox can never be answered.
 * The frame class's procedure posts those commands back to the message loop
 * and restores the window before FrameWndProc sees them, so the box shows
 * over the restored frame and gets input.
 */

#define WM_STARS_SYSCOMMAND (WM_APP + 1)

LRESULT CALLBACK FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// FrameWndProc16 defers FrameWndProc's restore and maximize commands to the
// message loop and passes every other message straight through.
static inline LRESULT CALLBACK FrameWndProc16(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_RESTORE || (wParam & 0xfff0) == SC_MAXIMIZE) {
            PostMessage(hwnd, WM_STARS_SYSCOMMAND, wParam, lParam);
            return 0;
        }
        break;
    case WM_STARS_SYSCOMMAND:
        if (IsIconic(hwnd))
            ShowWindow(hwnd, SW_RESTORE);
        return FrameWndProc(hwnd, WM_SYSCOMMAND, wParam, lParam);
    }
    return FrameWndProc(hwnd, msg, wParam, lParam);
}

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

// Application messages (WM_USER + 0x64...).
#define WM_STARS_STARTUP  0x0464
#define WM_STARS_HOST     0x0465
#define WM_STARS_CONTINUE 0x0466

#endif
