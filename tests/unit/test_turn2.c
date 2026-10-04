#include "acutest.h"

#include "stars_test.h"

// AddDetonatingField adds iplr's detonating standard minefield of radius
// 100 ly centered on pt.
static void AddDetonatingField(int16_t iplr, POINT16 pt) {
    THING *lpth;

    lpth = LpthNew(iplr, ithMinefield);
    lpth->pt = pt;
    lpth->thm.cMines = 10000;
    lpth->thm.iType = mineStandard;
    lpth->thm.fDetonate = TRUE;
    lpth->thm.grbitPlr = 1 << iplr;
}

// AddScout adds one ship of player 0's first design at its homeworld and
// returns the fleet's id; *pish is the design.
static int16_t IdflAddScout(int16_t *pish) {
    PLANET *lppl;
    FLEET  *lpfl;
    int16_t ish;

    lppl = LpplStarsTestHomeworld(0);
    if (lppl == NULL)
        return -1;
    for (ish = 0; ish < 16 && rglpshdef[0][ish].fFree; ish++) {
    }
    if (ish == 16)
        return -1;
    lpfl = LpflStarsTestAddFleet(0, lppl->id, ish, 1);
    lpfl->pt = rgptPlan[lppl->id];
    *pish = ish;
    return lpfl->id;
}

// A detonating minefield over a fleet crashed the native build:
// FTravelThroughMineFields read the travel distance through ThingDecay's
// NULL pointer, which Win16 read harmlessly from the start of its data
// segment.
static void test_ThingDecay_detonation_damages_fleet(void) {
    char        szDir[MAX_PATH];
    const char *rgszAi[] = {"#1 4"};
    FLEET      *lpfl;
    int16_t     ish;
    int16_t     idfl;

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir("ThingDecay_detonation", szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, rgszAi, 1));
    TEST_ASSERT(FStarsTestLoadHost());
    idfl = IdflAddScout(&ish);
    TEST_ASSERT(idfl >= 0);
    lpfl = LpflFromId(idfl);
    AddDetonatingField(1, lpfl->pt);

    ThingDecay();
    lpfl = LpflFromId(idfl);
    TEST_ASSERT(lpfl != NULL);
    TEST_CHECK_(lpfl->fDead || lpfl->rgdv[ish].dp != 0, "the detonating field did no damage");
}

TEST_LIST = {{"ThingDecay detonation damages a fleet", test_ThingDecay_detonation_damages_fleet}, {NULL, NULL}};
