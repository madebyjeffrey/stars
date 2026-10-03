#ifndef STARS_TEST_H
#define STARS_TEST_H

// Helpers for unit tests that link the game code. Acutest runs every test
// in its own process, so each test starts from the game's initial globals
// and calls FStarsTestInit first. Helpers return FALSE on failure.

#include "common.h"

// FStarsTestInit prepares the game state WinMain sets up before it creates
// windows: the instance, brushes and fonts (FCreateStuff), and a fixed RNG
// seed.
int16_t FStarsTestInit(void);

// FStarsTestDir creates, or empties, work\<pszName> under the test working
// directory and returns its absolute Windows path in szDir.
int16_t FStarsTestDir(const char *pszName, char *szDir, size_t cchDir);

// FStarsTestNewGame creates a game in szDir from a one-player tiny universe
// definition (the tutorial-sized test race, seed lSeed) and sets szBase to
// it. rgszAi lists extra players as "#<race> <skill>" definition lines.
int16_t FStarsTestNewGame(const char *szDir, uint32_t lSeed, const char **rgszAi, int16_t cAi);

// FStarsTestLoadHost unloads the current game and loads szBase's host file.
int16_t FStarsTestLoadHost(void);

// FStarsTestGenerate generates one turn from szBase's host file and turn
// files, as the -g command line does.
int16_t FStarsTestGenerate(void);

#endif
