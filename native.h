#ifndef STARS_NATIVE_H
#define STARS_NATIVE_H

// Helpers the Win32 build needs where Stars' Win16 types and behavior differ
// from Win32. See docs/NATIVE-PORT.md.

#include <limits.h>
#include <stdlib.h>

/*
 * Win16 points
 *
 * Win16 POINT held 16-bit ints, and Stars writes records holding points to
 * its files. Stars' points are POINT16 to keep that layout; Win32 POINT holds
 * LONGs, so points convert where they pass into or out of the Win32 API
 * (nativeui.h).
 */

typedef struct tagPOINT16 {
    int16_t x;
    int16_t y;
} POINT16;

// qsort16 sorts like Stars' Win16 CRT qsort, which decides the order of
// elements with equal keys. Turn generation depends on that order.
void qsort16(void *base, size_t count, size_t width, int (*compare)(const void *, const void *));

/*
 * Files
 *
 * The game reads and writes one file at a time through hf, a small handle
 * that Win16 OpenFile returned (file.c). These keep that interface on every
 * platform: Win32 keeps the original calls; POSIX has no share modes, so a
 * file another program has open is not refused there.
 */

// HfOpenFile opens szFile with Win16 OpenFile flags (mdRead and the like)
// and returns its handle, or -1 with *pfMissing set if it does not exist.
int16_t HfOpenFile(char *szFile, uint16_t mdOpen, int16_t *pfMissing);
int32_t CbReadFile(int16_t hf, void *rg, uint16_t cb);
int32_t CbWriteFile(int16_t hf, void *rg, uint16_t cb);
// LSeekFile moves like _llseek: iOrigin 0 from the start, 1 from the
// current position, 2 from the end. It returns the new position.
int32_t LSeekFile(int16_t hf, int32_t l, int16_t iOrigin);
int32_t CbFileSize(int16_t hf);
void    CloseFile(int16_t hf);
int16_t FFileExists(char *szFile);
int16_t FFileReadOnly(char *szFile);
void    MakeDir(char *szDir);

// File names: the directory separator.
#ifdef _WIN32
#define chDirSep '\\'
#define szDirSep "\\"
#else
#define chDirSep '/'
#define szDirSep "/"
#endif

// DwTickCount counts milliseconds like GetTickCount, for retries and file
// salts.
uint32_t DwTickCount();
// GetDateTimeSz writes the date as MM/DD/YY and the time as HH:MM:SS, as
// _strdate and _strtime did, for the turn log.
void GetDateTimeSz(char *szDate, char *szTime);
// FSzPrefixNoCase compares the first cch characters, ignoring case.
int16_t FSzPrefixNoCase(char *sz1, char *sz2, int16_t cch);

/*
 * Formatting and arithmetic
 *
 * Win32 long is 32 bits, so the game's formats (and its string table) pass
 * int32_t values to %ld. CchSprintf formats like wsprintf with l read as 32
 * bits on every platform. LMulDiv rounds like Win32 MulDiv.
 */
int     CchSprintf(char *sz, const char *szFormat, ...);
int32_t LMulDiv(int32_t lNumber, int32_t lNumerator, int32_t lDenominator);

#endif
