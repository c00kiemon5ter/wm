#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>

#include "global.h"

void monitor_focus(monitor_t *);
void monitor_focus_next(void);
void monitor_focus_prev(void);

bool randr(void);
bool xinerama(void);
void zaphod(void);

#endif
