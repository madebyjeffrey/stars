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

TEST_LIST = {{"generate one turn", test_generate_one_turn}, {NULL, NULL}};
