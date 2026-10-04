#include "stars_test.h"

#include <direct.h>

int  cStarsTestAlert;
char szStarsTestAlert[512];

int16_t __wrap_AlertSz(char *sz, int16_t mbType) {
    cStarsTestAlert++;
    snprintf(szStarsTestAlert, sizeof(szStarsTestAlert), "%s", sz);
    return (mbType & MB_TYPEMASK) == MB_YESNO || (mbType & MB_TYPEMASK) == MB_YESNOCANCEL ? IDYES : IDOK;
}

int16_t FStarsTestInit(void) {
    hInst = GetModuleHandle(NULL);
    szBase[0] = 0;
    ini.wFlags = 0;
    memset(&tutor, 0, sizeof(TUTOR));
    memset(&vtimer, 0, sizeof(TIMER));
    Randomize2(12345);
    fFileErrSilent = TRUE;
    idPlayer = iplrNone;
    return FCreateStuff();
}

int16_t FStarsTestDir(const char *pszName, char *szDir, size_t cchDir) {
    char szCwd[MAX_PATH];

    if (GetCurrentDirectoryA(sizeof(szCwd), szCwd) == 0)
        return FALSE;
    _mkdir("work");
    snprintf(szDir, cchDir, "%s\\work\\%s", szCwd, pszName);
    _mkdir(szDir);
    snprintf(szWork, sizeof(szWork), "%s\\game.*", szDir);
    {
        WIN32_FIND_DATAA fd;
        HANDLE           hfind;
        char             szFile[MAX_PATH];

        hfind = FindFirstFileA(szWork, &fd);
        if (hfind != INVALID_HANDLE_VALUE) {
            do {
                snprintf(szFile, sizeof(szFile), "%s\\%s", szDir, fd.cFileName);
                DeleteFileA(szFile);
            } while (FindNextFileA(hfind, &fd));
            FindClose(hfind);
        }
    }
    return TRUE;
}

// FCopyRace copies the test race, which CMake places in data/, next to the game.
static int16_t FCopyRace(const char *szDir) {
    char szDst[MAX_PATH];

    snprintf(szDst, sizeof(szDst), "%s\\human.r1", szDir);
    return CopyFileA("data\\humanoid.r1", szDst, FALSE) != 0;
}

int16_t FStarsTestNewGame(const char *szDir, uint32_t lSeed, const char **rgszAi, int16_t cAi) {
    char    szDef[MAX_PATH];
    FILE   *fp;
    int16_t i;

    if (!FCopyRace(szDir))
        return FALSE;
    snprintf(szDef, sizeof(szDef), "%s\\game.def", szDir);
    fp = fopen(szDef, "wb");
    if (fp == NULL)
        return FALSE;
    // Tiny universe, sparse, distant positions; all game options off.
    fprintf(fp, "Unit Test\r\n0 0 1 %lu\r\n0 0 0 0 0 1 0\r\n%d\r\n%s\\human.r1\r\n", (unsigned long)lSeed, 1 + cAi, szDir);
    for (i = 0; i < cAi; i++)
        fprintf(fp, "%s\r\n", rgszAi[i]);
    // Victory conditions off.
    for (i = 0; i < 8; i++)
        fprintf(fp, "0\r\n");
    fprintf(fp, "%s\\game.xy\r\n", szDir);
    fclose(fp);

    if (!GenNewGameFromFile(szDef))
        return FALSE;
    DestroyCurGame();
    snprintf(szBase, sizeof(szBase), "%s\\game", szDir);
    return TRUE;
}

int16_t FStarsTestLoadHost(void) {
    DestroyCurGame();
    idPlayer = iplrNone;
    return FLoadGame(szBase, "hst");
}

int16_t FStarsTestLoadPlayer(int16_t iPlr) {
    char szExt[4];

    DestroyCurGame();
    idPlayer = iPlr;
    snprintf(szExt, sizeof(szExt), "m%d", iPlr + 1);
    return FLoadGame(szBase, szExt);
}

PLANET *LpplStarsTestHomeworld(int16_t iPlr) {
    int16_t i;

    for (i = 0; i < cPlanet; i++) {
        if (lpPlanets[i].iPlayer == iPlr && lpPlanets[i].fHomeworld)
            return &lpPlanets[i];
    }
    return NULL;
}

FLEET *LpflStarsTestAddFleet(int16_t iPlr, int16_t idPlanet, int16_t ishdef, int16_t csh) {
    FLEET *lpfl;

    lpfl = LpflNew(iPlr, idPlanet);
    lpfl->rgcsh[ishdef] = csh;
    rglpshdef[iPlr][ishdef].cExist += csh;
    rglpshdef[iPlr][ishdef].cBuilt += csh;
    return lpfl;
}

int16_t FStarsTestSaveHost(void) {
    idPlayer = iplrNone;
    return FWriteDataFile(szBase, -1, FALSE);
}

int16_t FStarsTestSaveGame(void) {
    int16_t i;

    for (i = 0; i < game.cPlayer; i++) {
        idPlayer = iplrNone;
        if (!FWriteDataFile(szBase, i, FALSE))
            return FALSE;
    }
    return FStarsTestSaveHost();
}

int16_t FStarsTestGenerate(void) {
    DestroyCurGame();
    idPlayer = iplrNone;
    EnsureAis();
    return FGenerateTurn();
}
