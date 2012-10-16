#ifndef ICCCM_H
#define ICCCM_H

#include <stdbool.h>
#include <xcb/xcb.h>

bool icccm_get_window_name(const xcb_window_t, char *);
bool icccm_is_transient(const xcb_window_t);

#endif
