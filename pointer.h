#ifndef POINTER_H
#define POINTER_H

#include <stdint.h>
#include <stdbool.h>

#define POINTER_RESET       68
#define POINTER_TCORSS      130

bool pointer_grab(xcb_cursor_t);
void pointer_ungrab(void);
xcb_cursor_t pointer_get_by_id(const uint16_t);

#endif
