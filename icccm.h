#ifndef ICCCM_H
#define ICCCM_H

#include <stdbool.h>

#include <xcb/xcb.h>

#define WM_PROTOCOLS        "WM_PROTOCOLS"
#define WM_DELETE_WINDOW    "WM_DELETE_WINDOW"

bool icccm_close_window(const xcb_window_t);
bool icccm_get_window_name(const xcb_window_t, char *);
bool icccm_is_transient(const xcb_window_t);

#endif
