#include "acutest.h"

#include "stars_test.h"

// SpaceDockWithArmor returns a Space Dock design with cArmor Superlatanium
// (1500 dp each) in its shield/armor slot.
static SHDEF SpaceDockWithArmor(int16_t cArmor) {
    SHDEF shdef;

    memset(&shdef, 0, sizeof(shdef));
    shdef.hul = LphuldefFromId(ihuldefSpaceDock)->hul;
    shdef.det = detAll;
    shdef.hul.rghs[2].grhst = hstArmor;
    shdef.hul.rghs[2].iItem = iarmorSuperlatanium;
    shdef.hul.rghs[2].cItem = cArmor;
    return shdef;
}

// LoadPlayer loads player 1's turn file of a new tiny game, with or without
// Regenerating Shields.
static void LoadPlayer(const char *pszTest, int16_t fRS) {
    char szDir[MAX_PATH];

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir(pszTest, szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, NULL, 0));
    TEST_ASSERT(FStarsTestLoadPlayer(0));
    SetRaceGrbit(&rgplr[0], ibitRaceRegeneratingShields, fRS);
}

static void test_UpdateShdefCost_armor(void) {
    SHDEF shdef;

    LoadPlayer("UpdateShdefCost_armor", FALSE);
    shdef = SpaceDockWithArmor(20);
    UpdateShdefCost(&shdef);
    TEST_CHECK_(shdef.hul.dp == 250 + 30000, "dp %u", shdef.hul.dp);
    shdef = SpaceDockWithArmor(24);
    UpdateShdefCost(&shdef);
    TEST_CHECK_(shdef.hul.dp == 250 + 36000, "dp %u", shdef.hul.dp);
}

// Space Dock armor overflow: with RS the armor slot's strength was halved
// after it had overflowed signed 16 bits, so 24 Superlatanium (36000 dp)
// gave the hull about 50000 dp instead of 18250.
static void test_UpdateShdefCost_rs_armor_overflow(void) {
    SHDEF shdef;

    LoadPlayer("UpdateShdefCost_rs_armor_overflow", TRUE);
    shdef = SpaceDockWithArmor(20);
    UpdateShdefCost(&shdef);
    TEST_CHECK_(shdef.hul.dp == 250 + 15000, "dp %u", shdef.hul.dp);
    shdef = SpaceDockWithArmor(24);
    UpdateShdefCost(&shdef);
    TEST_CHECK_(shdef.hul.dp == 250 + 18000, "dp %u", shdef.hul.dp);
}

TEST_LIST = {{"UpdateShdefCost armor", test_UpdateShdefCost_armor},
             {"UpdateShdefCost RS armor overflow", test_UpdateShdefCost_rs_armor_overflow},
             {NULL, NULL}};
