#include "acutest.h"

#include "stars_test.h"

// OpenBattlePlans loads player 1's turn file and opens the real Battle Plans
// dialog on its first plan.
static HWND OpenBattlePlans(const char *pszTest) {
    char szDir[MAX_PATH];
    HWND hwnd;

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir(pszTest, szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, NULL, 0));
    TEST_ASSERT(FStarsTestLoadPlayer(0));
    hwnd = CreateDialogParamA(hInst, MAKEINTRESOURCE(IDD_BATTLE_PLANS), NULL, BattlePlansDlg, 0);
    TEST_ASSERT(hwnd != NULL);
    return hwnd;
}

// ChooseIn selects entry i of drop-down idc and notifies the dialog, as a
// player's choice does.
static void ChooseIn(HWND hwnd, int idc, int i) {
    HWND hwndCtl;

    hwndCtl = GetDlgItem(hwnd, idc);
    TEST_ASSERT(hwndCtl != NULL);
    TEST_ASSERT(SendMessage(hwndCtl, CB_SETCURSEL, i, 0) == i);
    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(idc, CBN_SELCHANGE), (LPARAM)hwndCtl);
}

// The Battle Plans drop-downs read their selection with the Win16 message
// number for CB_GETCURSEL, which Win32 combo boxes ignore, so every choice
// was stored as 0.
static void test_BattlePlansDlg_stores_choices(void) {
    HWND hwnd;

    hwnd = OpenBattlePlans("BattlePlansDlg_stores_choices");
    ChooseIn(hwnd, IDC_BATTLE_PLAN_PRIMARY_TARGET, 2);
    TEST_CHECK_(btlplan.mdTarget1 == 2, "primary target %d", btlplan.mdTarget1);
    ChooseIn(hwnd, IDC_BATTLE_PLAN_SECONDARY_TARGET, 3);
    TEST_CHECK_(btlplan.mdTarget2 == 3, "secondary target %d", btlplan.mdTarget2);
    ChooseIn(hwnd, IDC_BATTLE_PLAN_TACTIC, 1);
    TEST_CHECK_(btlplan.mdTactic == 1, "tactic %d", btlplan.mdTactic);
    DestroyWindow(hwnd);
}

TEST_LIST = {{"BattlePlansDlg stores drop-down choices", test_BattlePlansDlg_stores_choices}, {NULL, NULL}};
