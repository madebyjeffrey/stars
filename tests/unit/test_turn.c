#include "acutest.h"

#include "stars_test.h"

// test_generate_one_turn checks the turn-test plumbing: create a game, load
// its host file, generate a turn, and load the result.
static void test_generate_one_turn(void) {
    char        szDir[MAX_PATH];
    const char *rgszAi[] = {"#1 4"};

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir("generate_one_turn", szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, rgszAi, 1));

    TEST_ASSERT(FStarsTestLoadHost());
    TEST_CHECK_(game.turn == 0, "turn %d", game.turn);
    TEST_CHECK_(game.cPlayer == 2, "players %d", game.cPlayer);
    TEST_CHECK(cPlanet > 0);

    TEST_ASSERT(FStarsTestGenerate());
    TEST_ASSERT(FStarsTestLoadHost());
    TEST_CHECK_(game.turn == 1, "turn %d", game.turn);
}

// FReadFile reads up to cbMax bytes of szFile into rgb and returns the count
// in *pcb.
static int16_t FReadFile(const char *szFile, uint8_t *rgb, size_t cbMax, size_t *pcb) {
    FILE *fp;

    fp = fopen(szFile, "rb");
    if (fp == NULL)
        return FALSE;
    *pcb = fread(rgb, 1, cbMax, fp);
    fclose(fp);
    return TRUE;
}

// When a player submits no turn, generation keeps a backup of the turn file
// the player last received. The copy's arguments were reversed (backup to
// live file, after the backup had been removed), so no backup was kept.
static void test_generate_backs_up_unsubmitted_turn(void) {
    char    szDir[MAX_PATH];
    char    szFile[MAX_PATH];
    uint8_t rgbBefore[16384];
    uint8_t rgbBackup[16384];
    size_t  cbBefore;
    size_t  cbBackup;

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir("generate_backs_up_unsubmitted_turn", szDir, sizeof(szDir)));
    TEST_ASSERT(FStarsTestNewGame(szDir, 12345, NULL, 0));
    snprintf(szFile, sizeof(szFile), "%s\\game.m1", szDir);
    TEST_ASSERT(FReadFile(szFile, rgbBefore, sizeof(rgbBefore), &cbBefore));

    TEST_ASSERT(FStarsTestGenerate());
    snprintf(szFile, sizeof(szFile), "%s\\backup\\game.m1", szDir);
    TEST_ASSERT_(FReadFile(szFile, rgbBackup, sizeof(rgbBackup), &cbBackup), "no %s", szFile);
    TEST_CHECK(cbBackup == cbBefore && memcmp(rgbBackup, rgbBefore, cbBefore) == 0);
}

TEST_LIST = {{"generate one turn", test_generate_one_turn},
             {"generate backs up an unsubmitted turn", test_generate_backs_up_unsubmitted_turn},
             {NULL, NULL}};
