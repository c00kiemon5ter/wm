#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>

#include "global.h"

void monitor_focus(monitor_t *);
bool randr(void);
bool xinerama(void);
void zaphod(void);

#endif
