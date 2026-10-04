// CMake links this test with --wrap=AlertSz so dumps don't open message
// boxes.

#include "acutest.h"

#include "stars_test.h"

int16_t __wrap_AlertSz(char *sz, int16_t mbType) { return IDOK; }

// The planet dump passed the 32-bit mine and factory counts as two 16-bit
// words each, as the Win16 binary pushed them, so the Factories column
// printed 0 and the factory count shifted into Def %.
static void test_DumpPlanets_mines_and_factories(void) {
    char    szDir[MAX_PATH];
    char    szFile[MAX_PATH];
    char    szLine[1024];
    char   *rgpszField[16];
    char   *psz;
    char    szName[64];
    PLANET *lppl;
    FILE   *fp;
    int     cField;
    int     fFound;

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir("DumpPlanets_mines_and_factories", szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, NULL, 0));
    TEST_ASSERT(FStarsTestLoadPlayer(0));
    lppl = LpplStarsTestHomeworld(0);
    TEST_ASSERT(lppl != NULL);
    TEST_ASSERT(lppl->cMines != lppl->cFactories || lppl->cMines != 0);
    strcpy(szName, PszGetCompressedPlanet(rgidPlan[lppl->id]));

    DumpPlanets();
    snprintf(szFile, sizeof(szFile), "%s.pla", szBase);
    fp = fopen(szFile, "rb");
    TEST_ASSERT_(fp != NULL, "open %s", szFile);
    fFound = FALSE;
    while (fgets(szLine, sizeof(szLine), fp)) {
        cField = 0;
        for (psz = strtok(szLine, "\t\r\n"); psz && cField < 16; psz = strtok(NULL, "\t\r\n"))
            rgpszField[cField++] = psz;
        if (cField < 10 || strcmp(rgpszField[0], szName) != 0)
            continue;
        fFound = TRUE;
        // Planet Name, Owner, Starbase Type, Report Age, Population, Value,
        // Production Queue, Mines, Factories, Def %.
        TEST_CHECK_(atol(rgpszField[7]) == lppl->cMines, "mines %s, expected %d", rgpszField[7], lppl->cMines);
        TEST_CHECK_(atol(rgpszField[8]) == lppl->cFactories, "factories %s, expected %d", rgpszField[8], lppl->cFactories);
        TEST_CHECK_(strchr(rgpszField[9], '%') != NULL, "def %s", rgpszField[9]);
    }
    fclose(fp);
    TEST_CHECK_(fFound, "no row for %s", szName);
}

TEST_LIST = {{"DumpPlanets mines and factories", test_DumpPlanets_mines_and_factories}, {NULL, NULL}};
