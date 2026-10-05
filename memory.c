#include "common.h"

HB *LphbAlloc(uint16_t cb, HeapType ht) {
    void *hmem;
    HB   *lphb;

    lphb = NULL;
    cb += sizeof(HB);
    if (cb < mphtcbAlloc[ht]) {
        cb = mphtcbAlloc[ht];
    }
    /* NATIVE: the original locked a zero-filled GlobalAlloc block. */
    hmem = calloc(1, cb);
    if (!hmem) {
        AlertSz(PszFormatIds(idsMemory, NULL), MB_ICONHAND);
        StarsLongJump(penvMem, -1);
    }
    lphb = (HB *)hmem;
    lphb->hmem = hmem;
    lphb->cbBlock = cb;
    lphb->cbSlop = cb - sizeof(HB);
    lphb->cbFree = cb - sizeof(HB);
    lphb->ibTop = sizeof(HB);
    lphb->ht = ht;
    lphb->lphbNext = rglphb[ht];
    rglphb[ht] = lphb;
    return lphb;
}

HB *LphbReAlloc(HB *lphb) {
    void    *hmem;
    HB      *lphbT;
    HB      *lphbNew;
    uint16_t cbCur;
    uint16_t cbGrow;

    if (!lphb) {
        return NULL;
    }
    hmem = lphb->hmem;
    cbCur = lphb->cbBlock;
    cbGrow = mphtcbAlloc[lphb->ht];
    if (cbCur >= 0xffdc)
        goto LReAllocOOM;
    if (cbCur > (uint16_t)(0xffdc - cbGrow)) {
        cbGrow = 0xffdc - cbCur;
    }
    /* NATIVE: GlobalReAlloc zero-filled the bytes it added. */
    hmem = realloc(hmem, (size_t)(lphb->cbBlock + cbGrow));
    if (!hmem) {
    LReAllocOOM:
        AlertSz(PszFormatIds(idsMemory, NULL), MB_ICONHAND);
        StarsLongJump(penvMem, -1);
    }
    memset((uint8_t *)hmem + cbCur, 0, cbGrow);
    lphbNew = (HB *)hmem;
    lphbNew->hmem = hmem;
    if (rglphb[lphbNew->ht] == lphb) {
        rglphb[lphbNew->ht] = lphbNew;
    } else {
        for (lphbT = rglphb[lphbNew->ht]; lphbT && lphbT->lphbNext != lphb; lphbT = lphbT->lphbNext) {
        }
        lphbT->lphbNext = lphbNew;
    }
    lphbNew->cbBlock += cbGrow;
    lphbNew->cbFree += cbGrow;
    lphbNew->cbSlop += cbGrow;
    return lphbNew;
}

void FreeHb(HB *lphb) {
    void *hmem;
    HB   *lphbNext;

    if (lphb) {
        for (; lphb; lphb = lphbNext) {
            lphbNext = lphb->lphbNext;
            hmem = lphb->hmem;
            free(hmem);
        }
    }
    return;
}

void ResetHb(HeapType ht) {
    HB *lphb;

    for (lphb = rglphb[ht]; lphb; lphb = lphb->lphbNext) {
        lphb->ibTop = sizeof(HB);
        lphb->cbSlop = lphb->cbBlock - sizeof(HB);
        lphb->cbFree = lphb->cbBlock - sizeof(HB);
    }
    return;
}

void *LpAlloc(uint16_t cb, HeapType ht) {
    int16_t  fFree;
    uint16_t cbItem;
    uint8_t *lpbPrev;
    uint8_t *lpbTop;
    HB      *lphb;
    uint8_t *lpb;

    lphb = rglphb[ht];
    cb = (cb + 3) & 0xfffe;
    while (lphb && lphb->cbFree < cb) {
    LTryNextBlock:
        lphb = lphb->lphbNext;
    }
    if (!lphb) {
        lphb = LphbAlloc(cb, ht);
    }
    lpbTop = (uint8_t *)lphb + lphb->ibTop;
    if (lphb->cbSlop >= cb) {
        RawStore16(lpbTop, cb - 2);
        lphb->ibTop += cb;
        lphb->cbFree -= cb;
        lphb->cbSlop -= cb;
        return lpbTop + 2;
    }
    lpb = (uint8_t *)(lphb + 1);
    while (lpb < lpbTop) {
        lpbPrev = lpb;
        fFree = RawLoad16(lpb) & 1;
        cbItem = RawLoad16(lpb) & 0xfffe;
        lpb += 2 + cbItem;
        if (fFree) {
            while (lpb < lpbTop && (RawLoad16(lpb) & 1) && lpb - lpbPrev < cb) {
                lpb += 2 + (RawLoad16(lpb) & 0xfffe);
            }
            cbItem = lpb - lpbPrev - 2;
            RawStore16(lpbPrev, cbItem | 1);
            if ((uint16_t)(cbItem + 2) >= cb) {
                RawStore16(lpbPrev, RawLoad16(lpbPrev) & 0xfffe);
                lpbPrev += 2;
                lphb->cbFree -= cbItem + 2;
                return lpbPrev;
            }
        }
    }
    goto LTryNextBlock;
}

HB *LphbFromLpHt(void *lp, HeapType ht) {
    HB *lphb;

    if ((int16_t)ht < htOrd || (int16_t)ht >= htCount) {
        return NULL;
    }
    for (lphb = rglphb[ht]; lphb && ((HB *)lp <= lphb || (uint8_t *)lp >= (uint8_t *)lphb + lphb->cbBlock); lphb = lphb->lphbNext) {
    }
    if (!lphb) {
        return NULL;
    }
    return lphb;
}

void *LpReAlloc(void *lp, uint16_t cb, HeapType ht) {
    void    *lpNew;
    HB      *lphb;
    uint16_t cbCur;
    uint16_t cbGrow;

    cbCur = RawLoad16((uint8_t *)lp - 0x2);
    cb = (cb + 1) & 0xfffe;
    cbGrow = cb - cbCur;
    if (cb <= cbCur) {
        return lp;
    }
    lphb = LphbFromLpHt(lp, ht);
LGrewHeap:
    if ((uint8_t *)lphb + lphb->ibTop == (uint8_t *)lp + cbCur && lphb->cbSlop >= cbGrow) {
        lphb->cbSlop -= cbGrow;
        lphb->cbFree -= cbGrow;
        lphb->ibTop += cbGrow;
        RawStore16((uint8_t *)lp - 0x2, cb);
    } else if (ht == htPlanets || ht == htThings) {
        lphb = LphbReAlloc(lphb);
        lp = (uint8_t *)lphb + (sizeof(HB) + 2);
        goto LGrewHeap;
    } else {
        lpNew = LpAlloc(cb, ht);
        memcpy(lpNew, lp, cbCur);
        FreeLp(lp, ht);
        lp = lpNew;
    }
    return lp;
}

void FreeLp(void *lp, HeapType ht) {
    uint16_t cbFree;
    HB      *lphb;

    if (lp) {
        lphb = LphbFromLpHt(lp, ht);
        cbFree = RawLoad16((uint8_t *)lp - 0x2) + 2;
        RawStore16((uint8_t *)lp - 0x2, RawLoad16((uint8_t *)lp - 0x2) | 1);
        lphb->cbFree += cbFree;
        if ((uint8_t *)lp - (uint8_t *)lphb + cbFree - 2 == lphb->ibTop) {
            lphb->ibTop -= cbFree;
            lphb->cbSlop += cbFree;
        }
    }
    return;
}

PL *LpplReAlloc(PL *lppl, uint16_t cAlloc) {
    lppl = LpReAlloc(lppl, lppl->cbItem * cAlloc + 4, lppl->ht);
    lppl->iMax = cAlloc;
    return lppl;
}

PL *LpplAlloc(uint16_t cbItem, uint16_t cAlloc, HeapType ht) {
    PL *lppl;

    lppl = LpAlloc(cbItem * cAlloc + 4, ht);
    lppl->iMax = cAlloc;
    lppl->iMac = 0;
    lppl->fMark = FALSE;
    lppl->cbItem = cbItem;
    lppl->ht = ht;
    return lppl;
}

void FreePl(PL *lppl) {
    if (lppl) {
        FreeLp(lppl, lppl->ht);
    }
    return;
}
