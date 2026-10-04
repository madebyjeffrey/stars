#ifndef STARS_DECOMPILED_UTILUI_H
#define STARS_DECOMPILED_UTILUI_H

#include <stdint.h>
#include <windows.h>

void    SelectOursAtObject(POINT16 *ppt);
int16_t CchGetETA(HDC hdc, FLEET *lpfl, char *sz, int16_t iwp, int16_t fSmall);
void    DrawABunchOfStars(HDC hdc, RECT *prc);
void    DrawPlanetPrintDot(HDC hdc, int16_t x, int16_t y, int16_t iSize);

#endif
