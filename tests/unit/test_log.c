#include "acutest.h"

#include "stars_test.h"

// Mineral upload: minerals sent from a planet to another player's fleet
// were taken from the planet, but only what fit in the fleet's hold was
// delivered and the rest was destroyed.
static void test_FRunLogRecord_refunds_undelivered_cargo(void) {
    char        szDir[MAX_PATH];
    const char *rgszAi[] = {"#1 4"};
    PLANET     *lppl;
    FLEET      *lpfl;
    int16_t     ish;
    int16_t     idfl;
    int32_t     wtCargo;
    int32_t     wtPlanet;
    uint8_t     rgb[16];

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir("FRunLogRecord_refund", szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, rgszAi, 1));
    TEST_ASSERT(FStarsTestLoadHost());
    lppl = LpplStarsTestHomeworld(0);
    TEST_ASSERT(lppl != NULL);
    // One of the AI's ships that can carry cargo, orbiting player 0's
    // homeworld.
    for (ish = 0; ish < 16 && (rglpshdef[1][ish].fFree || WtMaxShdefStat(&rglpshdef[1][ish], 2) == 0); ish++) {
    }
    TEST_ASSERT(ish < 16);
    wtCargo = WtMaxShdefStat(&rglpshdef[1][ish], 2);
    TEST_ASSERT(wtCargo < 100);
    lpfl = LpflStarsTestAddFleet(1, lppl->id, ish, 1);
    idfl = lpfl->id;
    wtPlanet = lppl->rgwtMin[0];
    TEST_ASSERT(wtPlanet >= 100);

    // Player 0's order: 100 kT of ironium from the planet to the fleet.
    gd.fGeneratingTurn = TRUE;
    idPlayer = 0;
    memset(rgb, 0, sizeof(rgb));
    ((RTXFER *)rgb)->id1 = lppl->id;
    ((RTXFER *)rgb)->id2 = idfl;
    ((RTXFER *)rgb)->grobj1 = grobjPlanet;
    ((RTXFER *)rgb)->grobj2 = grobjFleet;
    ((RTXFER *)rgb)->grbitItems = 1;
    ((RTXFER *)rgb)->rgcQuan[0] = -100;
    TEST_ASSERT(FRunLogRecord(rtLogCargoXfer8, 7, rgb));

    lppl = LpplStarsTestHomeworld(0);
    lpfl = LpflFromId(idfl);
    TEST_CHECK_(lpfl->rgwtMin[0] == wtCargo, "fleet holds %ld of %ld", (long)lpfl->rgwtMin[0], (long)wtCargo);
    TEST_CHECK_(lppl->rgwtMin[0] == wtPlanet - wtCargo, "planet has %ld, had %ld", (long)lppl->rgwtMin[0], (long)wtPlanet);
}

TEST_LIST = {{"FRunLogRecord refunds undelivered cargo", test_FRunLogRecord_refunds_undelivered_cargo}, {NULL, NULL}};
