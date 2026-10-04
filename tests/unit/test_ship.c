#include "acutest.h"

#include "stars_test.h"

// FAddScoutPair loads a new one-player game and adds two fleets of one ship
// each of player 0's first design, orbiting its homeworld.
static int16_t FAddScoutPair(const char *pszTest, FLEET **plpflDst, FLEET **plpflSrc, int16_t *pishdef) {
    char    szDir[MAX_PATH];
    PLANET *lppl;
    int16_t ish;

    if (!FStarsTestInit() || !FStarsTestDir(pszTest, szDir, sizeof(szDir)) || !FStarsTestNewGame(szDir, 12345, NULL, 0) ||
        !FStarsTestLoadHost())
        return FALSE;
    lppl = LpplStarsTestHomeworld(0);
    if (lppl == NULL)
        return FALSE;
    for (ish = 0; ish < 16 && rglpshdef[0][ish].fFree; ish++) {
    }
    if (ish == 16)
        return FALSE;
    *plpflDst = LpflStarsTestAddFleet(0, lppl->id, ish, 1);
    *plpflSrc = LpflStarsTestAddFleet(0, lppl->id, ish, 1);
    *pishdef = ish;
    return TRUE;
}

// Repair after gating: a fleet that gated (or fought, or hit mines) can't
// heal this turn, but merging it by waypoint into a fleet that stayed put
// dropped its fNoHeal, so HealShips repaired the merged ships.
static void test_Merge2Fleets_keeps_no_heal(void) {
    FLEET  *lpflDst;
    FLEET  *lpflSrc;
    int16_t ishdef;

    TEST_ASSERT(FAddScoutPair("Merge2Fleets_no_heal", &lpflDst, &lpflSrc, &ishdef));
    lpflSrc->rgdv[ishdef].pctSh = 100;
    lpflSrc->rgdv[ishdef].pctDp = 100;
    lpflSrc->fNoHeal = TRUE;
    lpflDst->fHereAllTurn = TRUE;

    Merge2Fleets(lpflDst, lpflSrc, TRUE);
    TEST_ASSERT(lpflDst->rgcsh[ishdef] == 2);
    TEST_CHECK(lpflDst->fNoHeal);
    HealShips();
    TEST_CHECK_(lpflDst->rgdv[ishdef].pctDp == 100, "pctDp %d", lpflDst->rgdv[ishdef].pctDp);
}

TEST_LIST = {{"Merge2Fleets keeps no-heal", test_Merge2Fleets_keeps_no_heal}, {NULL, NULL}};
