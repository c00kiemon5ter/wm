#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include <stdbool.h>

#include <xcb/xcb.h>

void window_move(const xcb_window_t, const int16_t, const int16_t);
void window_resize(const xcb_window_t, const uint16_t, const uint16_t);
void window_move_resize(const xcb_window_t, const int16_t, const int16_t, const uint16_t, const uint16_t);
void window_move_resize_geom(const xcb_window_t, const xcb_rectangle_t);

bool window_update_geom(const xcb_window_t, xcb_rectangle_t *);
bool window_override_redirect(const xcb_window_t);
bool window_is_urgent(const xcb_window_t);
void window_set_border_width(const xcb_window_t, const uint16_t);

void window_show(const xcb_window_t);
void window_hide(const xcb_window_t);

void window_raise(const xcb_window_t);
void window_lower(const xcb_window_t);

#endif
