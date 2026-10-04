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
    TEST_ASSERT(FWasRaceFile("data\\humanoid.r1", FALSE));
    plr = vplr;
    strcpy(plr.szName, "Longname");
    strcpy(plr.szNames, "Longnames");
    // A shorter name typed over them, as GetDlgItemText leaves the buffers.
    strcpy(plr.szName, "Bo");
    strcpy(plr.szNames, "Bos");

    // Save as FSaveRace does.
    snprintf(szFile, sizeof(szFile), "%s\\short.r1", szDir);
    TEST_ASSERT(FCreateFile(dtRace, iplrNone, szFile));
    WriteRtPlr(&plr, NULL);
    icksum = IRaceChecksum(&plr);
    WriteRt(rtEOF, 2, &icksum);
    StreamClose();

    TEST_CHECK(FWasRaceFile(szFile, FALSE));
    TEST_CHECK(strcmp(vplr.szNames, "Bos") == 0);
}

TEST_LIST = {{"IRaceChecksum ignores stale name bytes", test_IRaceChecksum_ignores_stale_name_bytes}, {NULL, NULL}};
