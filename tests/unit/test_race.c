#include "acutest.h"

#include "stars_test.h"

// Race file corruption: the race wizard reads a name into a buffer that still
// holds a longer one, and IRaceChecksum included the stale bytes after the
// terminator. They aren't saved, so the file failed its checksum on load.
static void test_IRaceChecksum_ignores_stale_name_bytes(void) {
    char     szDir[MAX_PATH];
    char     szFile[MAX_PATH];
    PLAYER   plr;
    uint16_t icksum;

    TEST_ASSERT(FStarsTestInit());
    TEST_ASSERT(FStarsTestDir("IRaceChecksum_stale_name", szDir, sizeof(szDir)));
    TEST_ASSERT(FWasRaceFile("data" szDirSep "humanoid.r1", FALSE));
    plr = vplr;
    strcpy(plr.szName, "Longname");
    strcpy(plr.szNames, "Longnames");
    // A shorter name typed over them, as GetDlgItemText leaves the buffers.
    strcpy(plr.szName, "Bo");
    strcpy(plr.szNames, "Bos");

    // Save as FSaveRace does.
    snprintf(szFile, sizeof(szFile), "%s%sshort.r1", szDir, szDirSep);
    TEST_ASSERT(FCreateFile(dtRace, iplrNone, szFile));
    WriteRtPlr(&plr, NULL);
    icksum = IRaceChecksum(&plr);
    WriteRt(rtEOF, 2, &icksum);
    StreamClose();

    TEST_CHECK(FWasRaceFile(szFile, FALSE));
    TEST_CHECK(strcmp(vplr.szNames, "Bos") == 0);
}

// ReadRtPlr took the relation count and uncompressed name lengths from the
// record, so a corrupt one overran rgmdRelation into szName and the names
// into the rest of the PLAYER.
static void test_ReadRtPlr_bounds_record_lengths(void) {
    struct {
        PLAYER plr;
        char   rgchGuard[64];
    } buf;
    uint8_t rgb[300];
    char    szLong[33];
    int16_t iOff;
    int16_t cOut;
    int     i;

    // 40 relations, an uncompressed "Bob" and a 60-character plural name.
    memset(rgb, 0, sizeof(rgb));
    ((PLAYER *)rgb)->det = detAll;
    iOff = offsetof(PLAYER, rgmdRelation);
    rgb[iOff++] = 40;
    memset(&rgb[iOff], 1, 40);
    iOff += 40;
    iOff++;
    strcpy((char *)&rgb[iOff], "Bob");
    iOff += 4;
    memset(&rgb[iOff + 1], 'N', 60);
    wVersFile = 0;
    ((VERS *)&wVersFile)->verMajor = 2;
    ((VERS *)&wVersFile)->verMinor = 83;

    memset(&buf, 0x55, sizeof(buf));
    ReadRtPlr(&buf.plr, rgb);
    for (i = 0; i < 16; i++)
        TEST_CHECK(buf.plr.rgmdRelation[i] == 1);
    TEST_CHECK(strcmp(buf.plr.szName, "Bob") == 0);
    for (i = 4; i < (int)sizeof(buf.plr.szName); i++)
        TEST_CHECK(buf.plr.szName[i] == 0);
    TEST_CHECK(strlen(buf.plr.szNames) == sizeof(buf.plr.szNames) - 1);
    for (i = 0; i < (int)sizeof(buf.rgchGuard); i++)
        TEST_CHECK(buf.rgchGuard[i] == 0x55);

    // A compressed plural name of 32 characters, one more than a save holds.
    memset(szLong, 'a', 32);
    szLong[32] = 0;
    cOut = 64;
    TEST_ASSERT(FCompressUserString(szLong, (char *)&rgb[iOff + 1], &cOut));
    rgb[iOff] = (uint8_t)cOut;
    memset(&buf, 0x55, sizeof(buf));
    ReadRtPlr(&buf.plr, rgb);
    TEST_CHECK(buf.plr.szNames[0] == 0);
    for (i = 0; i < (int)sizeof(buf.rgchGuard); i++)
        TEST_CHECK(buf.rgchGuard[i] == 0x55);
}

#ifdef _WIN32
// Random race: the Random template is only the random-race bit, which
// GenerateWorld replaces with a random race. Choosing Custom on the first
// wizard page keeps the race being edited, so a race that had been Random
// kept the bit and was still replaced, however it was customized.
static void test_RaceWizardDlg1_custom_clears_random(void) {
    HWND hwnd;
    HWND hwndCtl;

    TEST_ASSERT(FStarsTestInit());
    vplr = vrgplrDef[6];
    TEST_ASSERT(GetRaceGrbit(&vplr, ibitRaceAIPlayer) != 0);
    strcpy(vplr.szName, "Randomite");
    strcpy(vplr.szNames, "Randomites");
    hwnd = CreateDialogParamA(hInst, MAKEINTRESOURCE(IDD_RACE_WIZARD_1), NULL, RaceWizardDlg1, 0);
    TEST_ASSERT(hwnd != NULL);
    CheckRadioButton(hwnd, IDC_RACE_HUMANOID, IDC_RACE_CUSTOM, IDC_RACE_CUSTOM);
    hwndCtl = GetDlgItem(hwnd, IDC_RACE_CUSTOM);
    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_RACE_CUSTOM, BN_CLICKED), (LPARAM)hwndCtl);
    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_NEXT, BN_CLICKED), (LPARAM)GetDlgItem(hwnd, IDC_NEXT));
    DestroyWindow(hwnd);
    TEST_CHECK_(GetRaceGrbit(&vplr, ibitRaceAIPlayer) == 0, "the custom race is still random");
}
#endif

TEST_LIST = {{"IRaceChecksum ignores stale name bytes", test_IRaceChecksum_ignores_stale_name_bytes},
             {"ReadRtPlr bounds record lengths", test_ReadRtPlr_bounds_record_lengths},
#ifdef _WIN32
             {"RaceWizardDlg1 Custom clears the random-race bit", test_RaceWizardDlg1_custom_clears_random},
#endif
             {NULL, NULL}};
