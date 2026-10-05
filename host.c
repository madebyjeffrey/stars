#include "common.h"

// stars-host: the game's host without its windows. It takes the Windows
// game's host command line (-a, -g, -b, -t, -v, -s, -p, -l; see
// ParseCmdLine) and exits with the same value. Waiting for turns (-w)
// needs the Windows game's timer and is refused.

// The user interface (ui.h). There is no player to ask: messages go to
// stderr, questions get the cautious answer, and windows are not drawn.

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

// PromptPassword refuses: a host takes a file's password with -p.
int16_t PromptPassword() { return FALSE; }

// PromptSaveGame leaves a player's unsaved orders unsaved.
void PromptSaveGame() { return; }

void UpdateProgressGauge(ProgressStep pctX10) { return; }

void CreateChildWindows() { return; }

void PostOpenGame() { return; }

void ShowTitleScreen() { return; }

void AddMRUFile(char *pszFileName, char *pszExt) { return; }

void CloseGameWindows() { return; }

int16_t FFinishPlrMsgEntry(int16_t dInc) { return TRUE; }

void InvalidateReport(ReportType irpt, int16_t fReload) { return; }

void ShowPlanetSel() { return; }

void ShowFleetSel() { return; }

void ShowMainObjSel(int16_t fSameType, int16_t idSkip) { return; }

void ShowScanSel(int16_t fVis) { return; }

void ShowScanSelChange(SCAN *pscanOld, SCAN *pscan, int16_t fChgWp) { return; }

void ShowSelAt(POINT16 pt) { return; }

void FillShipDD(int16_t idSkip) { return; }

void FillFleetCompLB() { return; }

void FillSelProdLB() { return; }

void SetFleetDropDownSel(int16_t id) { return; }

void RedrawPlanShip(TileBits grbit) { return; }

void InvalidateMine() { return; }

void UpdateMsgTitle() { return; }

void AdvanceTutor() { return; }

// LpbLoadTutorLog has no tutorial to replay; the tutorial plays only in the
// Windows game.
uint8_t *LpbLoadTutorLog() { return NULL; }

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
