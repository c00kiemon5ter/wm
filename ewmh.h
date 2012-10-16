#ifndef EWMH_H
#define EWMH_H

#include <stdbool.h>
#include <xcb/xcb.h>

void ewmh_init(void);
void ewmh_set_supported_atoms(void);
void ewmh_update_active_window(const xcb_window_t);
void ewmh_update_wm_name(const char *);
bool ewmh_get_window_name(const xcb_window_t, char *);
bool ewmh_wm_state_fullscreen(const xcb_window_t win);

#endif
