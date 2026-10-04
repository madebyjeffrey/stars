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

// 32k ships per fleet: fleet ship counts are int16_t. A waypoint merge added
// them without a limit, so 32000 + 1000 ships became a negative count.
static void test_Merge2Fleets_caps_ship_count(void) {
    FLEET  *lpflDst;
    FLEET  *lpflSrc;
    int16_t ishdef;

    TEST_ASSERT(FAddScoutPair("Merge2Fleets_caps", &lpflDst, &lpflSrc, &ishdef));
    lpflDst->rgcsh[ishdef] = 32000;
    lpflSrc->rgcsh[ishdef] = 1000;

    Merge2Fleets(lpflDst, lpflSrc, TRUE);
    TEST_CHECK_(lpflDst->rgcsh[ishdef] == 32767, "destination has %d", lpflDst->rgcsh[ishdef]);
    TEST_CHECK_(lpflSrc->rgcsh[ishdef] == 233, "source kept %d", lpflSrc->rgcsh[ishdef]);
    TEST_CHECK(!lpflSrc->fDead);
}

// Merging all fleets at a location clamped an overflowing count to 32766,
// losing the rest of the ships.
static void test_FFleetMergeAll_keeps_overflow_fleet(void) {
    FLEET   *lpflDst;
    FLEET   *lpflSrc;
    FLEET    fl;
    int16_t  ishdef;
    int16_t  idSrc;
    int16_t  rgifl[2];

    TEST_ASSERT(FAddScoutPair("FFleetMergeAll_overflow", &lpflDst, &lpflSrc, &ishdef));
    lpflDst->rgcsh[ishdef] = 32000;
    lpflSrc->rgcsh[ishdef] = 1000;
    idSrc = lpflSrc->id;
    rgifl[0] = lpflDst->id;
    rgifl[1] = idSrc;
    vrgiflMerge = rgifl;
    vcflMerge = 2;
    gd.fGeneratingTurn = TRUE;
    fl = *lpflDst;

    FFleetMergeAll(&fl);
    lpflDst = LpflFromId(rgifl[0]);
    lpflSrc = LpflFromId(idSrc);
    TEST_ASSERT(lpflDst != NULL);
    TEST_CHECK_(lpflDst->rgcsh[ishdef] == 32000, "destination has %d", lpflDst->rgcsh[ishdef]);
    TEST_CHECK_(lpflSrc != NULL && lpflSrc->rgcsh[ishdef] == 1000, "the 1000-ship fleet was merged");
}

TEST_LIST = {{"Merge2Fleets keeps no-heal", test_Merge2Fleets_keeps_no_heal},
             {"Merge2Fleets caps ship counts", test_Merge2Fleets_caps_ship_count},
             {"FFleetMergeAll keeps a fleet that would overflow", test_FFleetMergeAll_keeps_overflow_fleet},
             {NULL, NULL}};
