#include "common.h"

INT_PTR CALLBACK AskSaveDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return 1;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_SAVE:
        case IDC_NO_DON_T_SAVE:
        case IDC_SAVESUBMIT:
            EndDialog(hwnd, LOWORD(wParam) == IDC_NO_DON_T_SAVE ? 0 : LOWORD(wParam) == IDC_SAVESUBMIT ? -1 : 1);
            return 1;
        case IDC_HELP:
            WinHelp(hwnd, szHelpFile, HELP_CONTEXT, 1090);
            return 1;
        }
        /* fallthrough */
    case WM_DESTROY:
    default:
        return 0;
    }
}
