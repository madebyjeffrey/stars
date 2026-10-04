#ifndef STARS_DECOMPILED_COMMON_H
#define STARS_DECOMPILED_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>

// Raw Win16 storage is byte-addressed and may be unaligned, so scalar
// accesses through it copy bytes instead of dereferencing a cast pointer.
static inline uint16_t RawLoad16(const void *p) {
    uint16_t v;
    memcpy(&v, p, sizeof v);
    return v;
}
static inline uint32_t RawLoad32(const void *p) {
    uint32_t v;
    memcpy(&v, p, sizeof v);
    return v;
}
static inline void RawStore16(void *p, uint16_t v) { memcpy(p, &v, sizeof v); }
static inline void RawStore32(void *p, uint32_t v) { memcpy(p, &v, sizeof v); }

#include "enums.h"
#include "native.h"

// Application messages (WM_USER + 0x64...).
#define WM_STARS_STARTUP  0x0464
#define WM_STARS_HOST     0x0465
#define WM_STARS_CONTINUE 0x0466

// Native storage; the analysis model retains the original 18-byte layout.
typedef jmp_buf ENV;

// Comparator for qsort and bsearch, which call it through the native int
// return type.
typedef int (*QSORTCOMPARE)(const void *, const void *);

// Dereference the saved pointer to the native jump-buffer array.
#define StarsLongJump(env, value) longjmp(*(env), (value))

#include "structs.h"

#include "ai.h"
#include "ai2.h"
#include "ai3.h"
#include "ai4.h"
#include "aiutil.h"
#include "battle.h"
#include "battleui.h"
#include "build.h"
#include "buildui.h"
#include "create.h"
#include "createui.h"
#include "file.h"
#include "globals.h"
#include "init.h"
#include "log.h"
#include "mdi.h"
#include "memory.h"
#include "mine.h"
#include "msg.h"
#include "parts.h"
#include "planet.h"
#include "planetui.h"
#include "popup.h"
#include "produce.h"
#include "produceui.h"
#include "race.h"
#include "raceui.h"
#include "report.h"
#include "research.h"
#include "researchui.h"
#include "save.h"
#include "scan.h"
#include "ship.h"
#include "shipui.h"
#include "ship2.h"
#include "stars.h"
#include "strings.h"
#include "tb.h"
#include "thing.h"
#include "turn.h"
#include "turn2.h"
#include "turn3.h"
#include "tutor.h"
#include "tutor2.h"
#include "util.h"
#include "utilgen.h"
#include "utilgenui.h"
#include "vcr.h"

#endif
