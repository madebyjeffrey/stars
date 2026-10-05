#include "common.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

// qsortSwap16 exchanges complete elements, including native-width pointers.
static void qsortSwap16(unsigned char *a, unsigned char *b, size_t width) {
    while (width != 0) {
        --width;
        unsigned char value = a[width];
        a[width] = b[width];
        b[width] = value;
    }
}

// qsort16 preserves the ordering of equal keys in Stars' Win16 CRT qsort.
// Reconstructed from 0024:0866-09c6, matching qsort.asm in C700 and MSVC
// MLIBCEW.LIB. The initial sorted check, first-element pivot, and strict
// partition boundaries matter: ICompLong compares planet X coordinates only.
void qsort16(void *base, size_t count, size_t width, int (*compare)(const void *, const void *)) {
    if (count < 2 || width == 0)
        return;

    unsigned char *items = (unsigned char *)base;
    size_t         i;
    for (i = 1; i < count; ++i) {
        if ((int16_t)compare(items + i * width, items + (i - 1) * width) < 0)
            break;
    }
    if (i == count)
        return;

    // Process the smaller partition first, bounding pending ranges by log2(count).
    size_t lows[sizeof(size_t) * CHAR_BIT], highs[sizeof(size_t) * CHAR_BIT];
    size_t pending = 0, lo = 0, hi = count - 1;
    for (;;) {
        if (lo < hi) {
            size_t left = lo, right = hi + 1;
            size_t lastLess = lo, firstGreater = hi;
            for (;;) {
                while (++left != hi) {
                    int order = (int16_t)compare(items + left * width, items + lo * width);
                    if (order > 0)
                        break;
                    if (order < 0)
                        lastLess = left;
                }
                for (;;) {
                    --right;
                    int order = (int16_t)compare(items + lo * width, items + right * width);
                    if (order > 0)
                        break;
                    if (order < 0)
                        firstGreater = right;
                    else if (right == lo)
                        break;
                }
                if (right <= left)
                    break;
                qsortSwap16(items + left * width, items + right * width, width);
                firstGreater = right;
                lastLess = left;
            }
            qsortSwap16(items + lo * width, items + right * width, width);

            if (hi - firstGreater >= lastLess - lo) {
                lows[pending] = firstGreater;
                highs[pending++] = hi;
                hi = lastLess;
            } else {
                lows[pending] = lo;
                highs[pending++] = lastLess;
                lo = firstGreater;
            }
        } else {
            if (pending == 0)
                return;
            lo = lows[--pending];
            hi = highs[pending];
        }
    }
}

#ifdef _WIN32

int16_t HfOpenFile(char *szFile, uint16_t mdOpen, int16_t *pfMissing) {
    OFSTRUCT of;
    int16_t  hf;

    hf = OpenFile(szFile, &of, mdOpen);
    *pfMissing = hf == -1 && of.nErrCode == 2;
    return hf;
}

int32_t CbReadFile(int16_t hf, void *rg, uint16_t cb) { return _lread(hf, rg, cb); }

int32_t CbWriteFile(int16_t hf, void *rg, uint16_t cb) { return _lwrite(hf, rg, cb); }

int32_t LSeekFile(int16_t hf, int32_t l, int16_t iOrigin) { return _llseek(hf, l, iOrigin); }

int32_t CbFileSize(int16_t hf) { return (int32_t)GetFileSize((HANDLE)(INT_PTR)hf, NULL); }

void CloseFile(int16_t hf) { _lclose(hf); }

int16_t FFileExists(char *szFile) { return _access(szFile, 0) != -1; }

int16_t FFileReadOnly(char *szFile) { return _access(szFile, 2) != 0; }

void MakeDir(char *szDir) { _mkdir(szDir); }

uint32_t DwTickCount() { return GetTickCount(); }

void GetDateTimeSz(char *szDate, char *szTime) {
    _strdate(szDate);
    _strtime(szTime);
}

int16_t FSzPrefixNoCase(char *sz1, char *sz2, int16_t cch) { return _strnicmp(sz1, sz2, cch) == 0; }

#else

int16_t HfOpenFile(char *szFile, uint16_t mdOpen, int16_t *pfMissing) {
    int fd;
    int grf;

    // OF_READ, OF_WRITE, OF_READWRITE; OF_CREATE truncates. Share modes
    // (0x0070) have no POSIX counterpart.
    switch (mdOpen & 3) {
    case 1:
        grf = O_WRONLY;
        break;
    case 2:
        grf = O_RDWR;
        break;
    default:
        grf = O_RDONLY;
        break;
    }
    if (mdOpen & 0x1000) {
        grf |= O_CREAT | O_TRUNC;
    }
    fd = open(szFile, grf, 0666);
    *pfMissing = fd == -1 && errno == ENOENT;
    if (fd > SHRT_MAX) {
        close(fd);
        fd = -1;
    }
    return (int16_t)fd;
}

int32_t CbReadFile(int16_t hf, void *rg, uint16_t cb) { return (int32_t)read(hf, rg, cb); }

int32_t CbWriteFile(int16_t hf, void *rg, uint16_t cb) { return (int32_t)write(hf, rg, cb); }

int32_t LSeekFile(int16_t hf, int32_t l, int16_t iOrigin) { return (int32_t)lseek(hf, l, iOrigin == 0 ? SEEK_SET : iOrigin == 1 ? SEEK_CUR : SEEK_END); }

int32_t CbFileSize(int16_t hf) {
    struct stat st;

    if (fstat(hf, &st) != 0) {
        return -1;
    }
    return (int32_t)st.st_size;
}

void CloseFile(int16_t hf) { close(hf); }

int16_t FFileExists(char *szFile) { return access(szFile, F_OK) == 0; }

int16_t FFileReadOnly(char *szFile) { return access(szFile, W_OK) != 0; }

void MakeDir(char *szDir) { mkdir(szDir, 0777); }

uint32_t DwTickCount() {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)((uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000);
}

void GetDateTimeSz(char *szDate, char *szTime) {
    time_t     t;
    struct tm *ptm;

    t = time(NULL);
    ptm = localtime(&t);
    strftime(szDate, 9, "%m/%d/%y", ptm);
    strftime(szTime, 9, "%H:%M:%S", ptm);
}

int16_t FSzPrefixNoCase(char *sz1, char *sz2, int16_t cch) { return strncasecmp(sz1, sz2, cch) == 0; }

#endif
