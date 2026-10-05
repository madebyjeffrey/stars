#include "win.h"

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
