#include "common.h"

// stars-host: the game's host without its windows. It takes the Windows
// game's host command line (-a, -g, -b, -t, -v, -s, -p, -l; see
// ParseCmdLine) and exits with the same value. Waiting for turns (-w)
// needs the Windows game's timer and is refused.

// IdAlertBox shows ui.h's messages: there is no player to ask, so they go to
// stderr and questions get the cautious answer. hostui.c has the rest of
// ui.h.

int16_t IdAlertBox(char *sz, int16_t mbType) {
    fprintf(stderr, "stars-host: %s\n", sz);
    switch (mbType & 0x000f) {
    case MB_OKCANCEL:
        return IDCANCEL;
    case MB_YESNOCANCEL:
    case MB_YESNO:
        return IDNO;
    default:
        return IDOK;
    }
}

int main(int argc, char *argv[]) {
    char     szCmdLine[1024];
    int16_t  fSeed;
    uint32_t lSeed;
    int      i;

    // ParseCmdLine reads one string, as WinMain received it. File names
    // can't hold spaces, as in the Windows game.
    szCmdLine[0] = 0;
    for (i = 1; i < argc; i++) {
        if (strlen(szCmdLine) + strlen(argv[i]) + 2 > sizeof(szCmdLine)) {
            fprintf(stderr, "stars-host: command line too long\n");
            return 2;
        }
        if (i > 1) {
            strcat(szCmdLine, " ");
        }
        strcat(szCmdLine, argv[i]);
    }
    szBase[0] = 0;
    ini.wFlags = 0;
    ini.idPlayer = iplrNone;
    InitGameStuff();
    fSeed = FALSE;
    lSeed = 0;
    ParseCmdLine(szCmdLine, &fSeed, &lSeed);
    Randomize2(fSeed ? lSeed : DwTickCount());
    if (!ini.fCmdLine) {
        fprintf(stderr, "usage: stars-host [-s<seed>] [-p<password>] [-l] (-a game.def | -v game.hst | -g[n] [-t] game.hst | -b batchfile)\n");
        return 2;
    }
    idPlayer = iplrNone;
    ini.fCmdLine = FALSE;
    if (!FRunCmdLine()) {
        fprintf(stderr, "stars-host: nothing to do; -w and opening a game need the Windows game\n");
        return 2;
    }
    return vretExitValue;
}
