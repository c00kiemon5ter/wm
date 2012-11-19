#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>

#include "wm.h"

void monitor_focus(monitor_t *);
monitor_t *monitor_next(const monitor_t *);
monitor_t *monitor_prev(const monitor_t *);

bool randr(void);
bool xinerama(void);
void zaphod(void);

#endif
