#ifndef POINTER_H
#define POINTER_H

#include <stdint.h>
#include <stdbool.h>

#define POINTER_DEFAULT     68
#define POINTER_TCORSS      130

bool window_grab_pointer(xcb_cursor_t);
void window_ungrab_pointer(void);
xcb_cursor_t window_get_pointer(const uint16_t);

#endif
