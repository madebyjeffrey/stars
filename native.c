#include "common.h"

// qsortSwap16 exchanges complete elements, including native-width pointers.
static void qsortSwap16(unsigned char *a, unsigned char *b, size_t width) {
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
void qsort16(void *base, size_t count, size_t width, int (*compare)(const void *, const void *)) {
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

DWORD GetTextExtent(HDC hdc, LPCSTR str, int len) {
    SIZE size;

    if (!GetTextExtentPoint32A(hdc, str, len, &size))
        return 0;

    return MAKELONG((WORD)size.cx, (WORD)size.cy);
}

/*
 * Frame restore under Wine
 *
 * FrameWndProc handles SC_RESTORE and SC_MAXIMIZE before DefWindowProc
 * restores the window, and while a submitted turn waits it asks, in a task
 * modal MessageBox, whether to load the new turn or unsubmit. Win16 delivered
 * those commands from the message loop. Wine's macOS driver sends SC_RESTORE
 * from inside its handler for a Dock un-minimize and takes no more input
 * events until that send returns, so the MessageBox can never be answered.
 * FrameWndProcDeferred, the frame class's procedure, posts those commands
 * back to the message loop and restores the window before FrameWndProc sees
 * them, so the box shows over the restored frame and gets input.
 */

#define WM_STARS_SYSCOMMAND (WM_APP + 1)

// FrameWndProcDeferred defers FrameWndProc's restore and maximize commands
// to the message loop and passes every other message straight through.
LRESULT CALLBACK FrameWndProcDeferred(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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
