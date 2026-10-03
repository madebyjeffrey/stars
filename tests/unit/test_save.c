#include "acutest.h"

#include "stars_test.h"

// ScanCloakedStarbase puts a player-0 fleet with a dScanRange2 penetrating
// scanner 100 ly from player 1's homeworld, whose starbase design has
// lVisible lVis2, and returns what SetVisPFFleets lets player 0 see of it.
static DetType DetScanCloakedStarbase(const char *pszTest, int16_t dScanRange2, int32_t lVis2) {
    char        szDir[MAX_PATH];
    const char *rgszAi[] = {"#1 4"};
    PLANET     *lppl0;
    PLANET     *lppl1;
    FLEET      *lpfl;
    int16_t     ish;
    int16_t     i;

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir(pszTest, szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, rgszAi, 1));
    TEST_ASSERT(FStarsTestLoadHost());
    lppl0 = LpplStarsTestHomeworld(0);
    lppl1 = LpplStarsTestHomeworld(1);
    TEST_ASSERT(lppl0 != NULL && lppl1 != NULL && lppl1->fStarbase);

    for (ish = 0; ish < 16 && rglpshdef[0][ish].fFree; ish++) {
    }
    TEST_ASSERT(ish < 16);
    rglpshdef[0][ish].dScanRange = 0;
    rglpshdef[0][ish].dScanRange2 = dScanRange2;
    lpfl = LpflStarsTestAddFleet(0, lppl0->id, ish, 1);
    lpfl->idPlanet = idPlanetDeepSpace;
    lpfl->pt.x = rgptPlan[lppl1->id].x + 100;
    lpfl->pt.y = rgptPlan[lppl1->id].y;
    rglpshdefSB[1][lppl1->isb].lVisible = lVis2;

    for (i = 0; i < cPlanet; i++) {
        lpPlanets[i].fInclude = FALSE;
        lpPlanets[i].det = detNone;
    }
    gd.fGeneratingTurn = TRUE;
    SetVisPFFleets(0);
    return lppl1->fInclude ? (DetType)lppl1->det : detNone;
}

// A starbase cloaked to lVisible 2500 (seen at half range) 100 ly away is
// within a 600 ly scanner's reduced 300 ly.
static void test_SetVisPFFleets_cloaked_starbase(void) {
    DetType det;

    det = DetScanCloakedStarbase("SetVisPFFleets_cloaked_starbase", 600, 2500);
    TEST_CHECK_(det >= detSome, "det %d", det);
}

// ISB vs scanning: lRadius2 * lVis2 overflowed 32 bits for scanners over
// about 463 ly, so a lightly cloaked starbase was only obscured.
static void test_SetVisPFFleets_cloaked_starbase_long_range(void) {
    DetType det;

    det = DetScanCloakedStarbase("SetVisPFFleets_cloaked_starbase_long_range", 600, 9000);
    TEST_CHECK_(det >= detSome, "det %d", det);
}

TEST_LIST = {{"SetVisPFFleets sees a cloaked starbase", test_SetVisPFFleets_cloaked_starbase},
             {"SetVisPFFleets long-range scanner sees a cloaked starbase", test_SetVisPFFleets_cloaked_starbase_long_range},
             {NULL, NULL}};
