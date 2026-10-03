#ifdef STARS_TEST_NATIVE_PORTS
#include <assert.h>
#include <stdlib.h>
#include "common.h"

static uint8_t rgRecord[4][1024];
static int16_t rgcbRecord[4];
static int     cRecord;
static int     iRead;
static int     fLogRead;

void __wrap_WriteRt(RecordType rt, int16_t cb, void *rg) {
    if (rt != rtPlrMsg)
        return;
    assert(cRecord < 4 && cb >= 12 && cb <= 1023);
    rgcbRecord[cRecord] = cb;
    memcpy(rgRecord[cRecord++], rg, cb);
}

int16_t __wrap_FCreateFile(DtFileType dt, int16_t iPlayer, char *psz) { return TRUE; }
void    __wrap_StreamClose(void) {}
void    __wrap_StreamOpen(char *szFile, MdOpenFlags mdOpen) {}
void    __wrap_DirtyGame(int16_t fDirty) {}

void __wrap_ReadRt(void) {
    memset(rgbCur, 0, sizeof(rgbCur));
    if (fLogRead && iRead == -2) {
        hdrCur.rt = rtBOF;
        hdrCur.cb = 16;
        RawStore32(rgbCur + 4, game.lid);
        RawStore16(rgbCur + 10, game.turn);
        RawStore16(rgbCur + 14, game.wGen << 13);
    } else if (fLogRead && iRead == -1) {
        hdrCur.rt = 9;
        hdrCur.cb = 17;
    } else if (iRead < cRecord) {
        hdrCur.rt = rtPlrMsg;
        hdrCur.cb = rgcbRecord[iRead];
        memcpy(rgbCur, rgRecord[iRead], hdrCur.cb);
    } else {
        hdrCur.rt = rtEOF;
        hdrCur.cb = 0;
    }
    iRead++;
}

static MSGPLR *MakeMessage(int16_t from, int16_t to, int16_t len) {
    MSGPLR *mp;
    int     i;

    mp = malloc(sizeof(MSGPLR) + abs(len));
    assert(mp != NULL);
    mp->lpmsgplrNext = NULL;
    mp->iPlrFrom = from;
    mp->iPlrTo = to;
    mp->iInRe = 123;
    mp->cLen = len;
    for (i = 0; i < abs(len); i++)
        mp->rgbMsg[i] = (uint8_t)(i * 17 + from);
    return mp;
}

static void CheckRecord(int i, MSGPLR *mp) {
    assert(rgcbRecord[i] == abs(mp->cLen) + 12);
    assert(RawLoad32(rgRecord[i]) == 0);
    assert(memcmp(rgRecord[i] + 4, &mp->iPlrFrom, 8) == 0);
    assert(memcmp(rgRecord[i] + 12, mp->rgbMsg, abs(mp->cLen)) == 0);
}

static void CheckReadMessage(MSGPLR *actual, MSGPLR *expected) {
    assert(actual != NULL);
    assert(actual->iPlrFrom == expected->iPlrFrom);
    assert(actual->iPlrTo == expected->iPlrTo);
    assert(actual->iInRe == expected->iInRe);
    assert(actual->cLen == expected->cLen);
    assert(memcmp(actual->rgbMsg, expected->rgbMsg, abs(expected->cLen)) == 0);
}

static void TestMessages(void) {
    MSGPLR *mp;
    MSGPLR *mp2;
    MSGPLR *mp3;
    uint8_t rgbMsgBuffer[16];

    mp = MakeMessage(0, 0, 31);
    assert(sizeof(mp->iPlrFrom) == 2 && sizeof(mp->cLen) == 2);
    assert((uint8_t *)mp->rgbMsg - (uint8_t *)&mp->iPlrFrom == 8);
    mp2 = MakeMessage(1, 2, -1000);
    mp3 = MakeMessage(2, 3, 7);
    mp->lpmsgplrNext = mp2;
    mp2->lpmsgplrNext = mp3;
    vlpmsgplrOut = mp;
    vcmsgplrOut = 3;
    lpMsg = (int16_t *)rgbMsgBuffer;
    imemMsgCur = 0;
    imemLogCur = 0;
    idPlayer = -1;
    hdrPrev.rt = rtLogPlayerZpq1;

    cRecord = 0;
    WritePlayerMessages(1);
    assert(cRecord == 2);
    CheckRecord(0, mp);
    CheckRecord(1, mp2);
    cRecord = 0;
    WritePlayerMessages(0);
    assert(cRecord == 0);
    WritePlayerMessages(-1);
    assert(cRecord == 0);

    cRecord = 0;
    assert(FWriteLogFile("native-port-test", 0) == TRUE);
    assert(cRecord == 3);
    CheckRecord(0, mp);
    CheckRecord(1, mp2);
    CheckRecord(2, mp3);

    /* Old saves contain an arbitrary four-byte link. It must be ignored. */
    RawStore32(rgRecord[0], 0xdeadbeef);
    RawStore32(rgRecord[1], 0xffffffff);
    iRead = 0;
    fLogRead = FALSE;
    __wrap_ReadRt();
    vlpmsgplrIn = NULL;
    vcmsgplrIn = 0;
    cMsg = 0;
    ReadPlayerMessages();
    assert(vcmsgplrIn == 3);
    CheckReadMessage(vlpmsgplrIn, mp);
    CheckReadMessage(vlpmsgplrIn->lpmsgplrNext, mp2);
    CheckReadMessage(vlpmsgplrIn->lpmsgplrNext->lpmsgplrNext, mp3);
    assert(vlpmsgplrIn->lpmsgplrNext->lpmsgplrNext->lpmsgplrNext == NULL);

    vlpmsgplrOut = NULL;
    vcmsgplrOut = 0;
    iRead = -2;
    fLogRead = TRUE;
    assert(FLoadLogFile("native-port-test") == TRUE);
    assert(vcmsgplrOut == 3);
    CheckReadMessage(vlpmsgplrOut, mp);
    CheckReadMessage(vlpmsgplrOut->lpmsgplrNext, mp2);
    CheckReadMessage(vlpmsgplrOut->lpmsgplrNext->lpmsgplrNext, mp3);
    assert(vlpmsgplrOut->lpmsgplrNext->lpmsgplrNext->lpmsgplrNext == NULL);
    free(mp);
    free(mp2);
    free(mp3);
    FreeHb(rglphb[htPlrMsg]);
    rglphb[htPlrMsg] = NULL;
    vlpmsgplrIn = vlpmsgplrOut = NULL;
}

static void TestStaticColor(void) {
    hbrButtonFace = (HBRUSH)(uintptr_t)0x1234;
    assert(RandomSeedDlg(NULL, WM_CTLCOLORSTATIC, 0, 0x1234) == (INT_PTR)hbrButtonFace);
    assert(RandomSeedDlg(NULL, WM_CTLCOLORMSGBOX, 0, 0x60001) == 0);
}

static void TestBattleRollover(void) {
    FLEET    fl;
    uint16_t grfAttack[16];
    uint8_t *lpbStart;
    uint8_t *lpbScratch;
    TOK      rgtok[256];

    memset(&fl, 0, sizeof(fl));
    memset(grfAttack, 0, sizeof(grfAttack));
    fl.idPlanet = -1;
    fl.lpflNext = &fl;
    vrgtok = rgtok;
    lpbBattleLog = LpAlloc(0xffc8, htBattle);
    lpbBattleT = LpAlloc(0xffc8, htBattle);
    lpbStart = lpbBattleLog;
    lpbScratch = lpbBattleT;

    /* Win16: data starts at offset 18, leaving 65462 bytes to the limit. */
    lpbBattleCur = lpbStart + 65448;
    assert(FDoCoolBattle(&fl, 0, grfAttack, 0, 0) == 1);
    assert(lpbBattleT == lpbScratch);
    assert(lpbBattleCur == lpbStart + 65462);
    assert(((BTLDATA *)(lpbStart + 65448))->cbData == 14);
    assert(FDoCoolBattle(&fl, 0, grfAttack, 0, 0) == 1);
    assert(lpbBattleT == NULL);
    assert(RawLoad16(lpbStart + 65462) == 0xffff);
    assert(lpbBattleCur == lpbScratch + 14);
    FreeHb(rglphb[htBattle]);
    rglphb[htBattle] = NULL;
    lpbBattleLog = lpbBattleCur = lpbBattleT = NULL;
    vrgtok = NULL;
}

int main(void) {
    TestMessages();
    TestStaticColor();
    TestBattleRollover();
    puts("PASS: player-message writers/readers, legacy links, filtering, 1000-byte text, static colors, battle rollover");
    return 0;
}
#endif
