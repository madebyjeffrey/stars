#ifndef STARS_DECOMPILED_PLANET_H
#define STARS_DECOMPILED_PLANET_H

#include <stdint.h>
#include <windows.h>

void    DrawPlanShip(HDC hdc, TileBits grbit);
int16_t FGetBestDefensePart(PART *ppart);
char   *PszProductionETA(PLANET *lppl, PLPROD *lpplprod, ProdItemType iItem, int16_t *etaFirst, int16_t *etaLast);
void    SetPlanetTitleBar(HWND hwnd);
void    ChangeMainObjSel(GrobjClass grobjNew, int16_t iObjSel);
void    FillShipDD(int16_t idSkip);
void    SelectAdjPlanet(int16_t dInc, int16_t idPlanet);
int16_t IdFindAdjStarbase(int16_t idPlanet, int16_t fNext);
int16_t IBestTerraform(PLANET *lppl, int16_t fHelp);
char   *PszCalcEnvVar(EnvType iEnv, int16_t iVar);
char   *PszCalcGravity(int16_t iGravity);
void    FillPlanetProdLB(HWND hwnd, PLPROD *lpplprod, PLANET *lppl);
int16_t PctPlanetCapacity(PLANET *lppl);
int16_t PctPlanetOptValue(PLANET *lppl, int16_t iPlr);
int16_t PctPlanetDesirability(PLANET *lppl, int16_t iPlr);
int32_t CalcPlanetMaxPop(int16_t idpl, int16_t iplr);
int16_t CMaxMines(PLANET *lppl, int16_t iplr);
int16_t CMaxOperableMines(PLANET *lppl, int16_t iplr, int16_t fNextYear);
int16_t CMinesOperating(PLANET *lppl);
int16_t CFactoriesOperating(PLANET *lppl);
int16_t CMaxFactories(PLANET *lppl, int16_t iplr);
int16_t CMaxOperableFactories(PLANET *lppl, int16_t iplr, int16_t fNextYear);
int16_t CMaxDefenses(PLANET *lppl, int16_t iplr);
int16_t CMaxOperableDefenses(PLANET *lppl, int16_t iplr, int16_t fNextYear);
int16_t CResourcesAtPlanet(PLANET *lppl, int16_t iplr);
int16_t IWarpMAFromLppl(PLANET *lppl, int16_t *pfTwo);
int16_t StargateRangeFromLppl(PLANET *lppl, int16_t iplr, int16_t ish);
int16_t FProdIsTerra(PROD *lpprod);
int16_t IpctCanTerraformLppl(PLANET *lppl);
int16_t FCanTerraformLppl(PLANET *lppl, int16_t *rgEnvMin, int16_t *rgEnvMax, int16_t *rgEnvCost, int16_t fHelp);
void    UninhabitPlanet(PLANET *lppl);
int16_t PctCloakFromHuldef(HUL *lphul, int16_t iplr, int16_t *ppctSteal);

#endif
