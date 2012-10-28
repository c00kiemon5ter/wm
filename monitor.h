#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>

#include "global.h"

#define MONITOR_DEFAULT_TAG     0
#define MONITOR_DEFAULT_M_AREA  0
#define MONITOR_DEFAULT_M_WINS  1
#define MONITOR_DEFAULT_SPACER  0
#define MONITOR_DEFAULT_BORDER  3
#define MONITOR_DEFAULT_LAYOUT  GRID

void monitor_focus(monitor_t *);
void monitor_focus_next(void);
void monitor_focus_prev(void);

bool randr(void);
bool xinerama(void);
void zaphod(void);

#endif
