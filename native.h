#ifndef STARS_NATIVE_H
#define STARS_NATIVE_H

// Helpers the Win32 build needs where Stars' Win16 types and behavior differ
// from Win32. See docs/NATIVE-PORT.md.

#include <direct.h>
#include <io.h>
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

#endif
