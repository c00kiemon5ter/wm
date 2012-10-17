#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdint.h>

#include <xcb/xcb.h>

#include "global.h"

void window_set_visibility(const xcb_window_t, bool);
void window_hide(const xcb_window_t);
void window_show(const xcb_window_t);
void window_set_border_width(const xcb_window_t, const uint16_t);
void window_move(const xcb_window_t, const int16_t, const int16_t);
void window_resize(const xcb_window_t, const uint16_t, const uint16_t);
void window_move_resize(const xcb_window_t, const int16_t, const int16_t, const uint16_t, const uint16_t);
void window_move_resize_geom(const xcb_window_t, const xcb_rectangle_t);
void client_set_geom(client_t *, const int16_t, const int16_t, const uint16_t, const uint16_t);
void client_set_geom_geom(client_t *, const xcb_rectangle_t);
void toggle_fullscreen(client_t *c);
bool window_override_redirect(const xcb_window_t);
bool window_update_geom(const xcb_window_t, xcb_rectangle_t *);
client_t *create_client(const xcb_window_t);
void add_client(client_t *);
client_t *locate(const xcb_window_t);

#endif
