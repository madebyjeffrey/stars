#include "acutest.h"

#include "stars_test.h"

// IshColonyDesign returns iPlr's first design carrying a colonization
// module and that module's slot in *ihs, or -1.
static int16_t IshColonyDesign(int16_t iPlr, int16_t *ihs) {
    int16_t ish;
    int16_t j;
    HUL    *lphul;

    for (ish = 0; ish < 16; ish++) {
        if (rglpshdef[iPlr][ish].fFree)
            continue;
        lphul = &rglpshdef[iPlr][ish].hul;
        for (j = 0; j < lphul->chs; j++) {
            if (lphul->rghs[j].grhst == hstSpecialM && lphul->rghs[j].iItem == ispecialMColonizationModule && lphul->rghs[j].cItem > 0) {
                *ihs = j;
                return ish;
            }
        }
    }
    return -1;
}

// FColonizeWith sends one ship of player 0's colony design, with its module
// count set to cModule, to colonize an empty planet with 100 kT of
// colonists, generates the turn, and reports whether player 0 owns the
// planet afterwards.
static int16_t FColonizeWith(const char *pszTest, int16_t cModule) {
    char    szDir[MAX_PATH];
    int16_t ish;
    int16_t ishNew;
    int16_t ihs;
    int16_t idPlanet;
    int16_t i;
    FLEET  *lpfl;
    ORDER  *lpord;

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir(pszTest, szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, NULL, 0));
    TEST_ASSERT(FStarsTestLoadHost());

    ish = IshColonyDesign(0, &ihs);
    TEST_ASSERT(ish >= 0);
    for (ishNew = 0; ishNew < 16 && !rglpshdef[0][ishNew].fFree; ishNew++) {
    }
    TEST_ASSERT(ishNew < 16);
    rglpshdef[0][ishNew] = rglpshdef[0][ish];
    rglpshdef[0][ishNew].ishdef = ishNew;
    rglpshdef[0][ishNew].cExist = 0;
    rglpshdef[0][ishNew].cBuilt = 0;
    rglpshdef[0][ishNew].hul.rghs[ihs].cItem = cModule;

    for (idPlanet = -1, i = 0; i < cPlanet; i++) {
        if (lpPlanets[i].iPlayer == iplrNone) {
            idPlanet = lpPlanets[i].id;
            break;
        }
    }
    TEST_ASSERT(idPlanet >= 0);
    lpfl = LpflStarsTestAddFleet(0, idPlanet, ishNew, 1);
    lpfl->rgwtMin[3] = 100;
    lpord = &lpfl->lpplord->rgord[0];
    lpord->grTask = grTaskColonize;
    lpord->fValidTask = TRUE;
    TEST_ASSERT(FStarsTestSaveHost());

    TEST_ASSERT(FStarsTestGenerate());
    TEST_ASSERT(FStarsTestLoadHost());
    return lpPlanets[idPlanet].iPlayer == 0;
}

static void test_colonize(void) { TEST_CHECK(FColonizeWith("colonize", 1)); }

// Colonization module check: the colonize task accepted a slot recorded as
// a colonization module even when it held none (cItem 0).
static void test_colonize_needs_module(void) { TEST_CHECK(!FColonizeWith("colonize_needs_module", 0)); }

TEST_LIST = {{"colonize", test_colonize}, {"colonize needs a colonization module", test_colonize_needs_module}, {NULL, NULL}};
