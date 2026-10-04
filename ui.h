#ifndef STARS_DECOMPILED_UI_H
#define STARS_DECOMPILED_UI_H

// The game code calls these to reach the player. The Windows game
// implements them in its UI files; stars-host implements them in host.c,
// where there is no one to ask. See docs/ROADMAP.md (2.9).

#include <stdint.h>

// Messages and questions.
int16_t IdAlertBox(char *sz, int16_t mbType);
int16_t PromptPassword();
void    PromptSaveGame();

// Progress during turn generation and universe creation.
void UpdateProgressGauge(ProgressStep pctX10);

// The tutorial.
void     AdvanceTutor();
uint8_t *LpbLoadTutorLog();

#endif
