#ifndef EWMH_H
#define EWMH_H

#include <xcb/xcb.h>

void ewmh_init(void);
void ewmh_set_supported_atoms(void);
void ewmh_update_wm_name(const char *wm_name);
void ewmh_update_active_window(const xcb_window_t win);
void ewmh_update_number_of_desktops(void);
void ewmh_update_current_desktop(void);
void ewmh_update_desktop_names(void);
void ewmh_update_client_list(void);

#endif
