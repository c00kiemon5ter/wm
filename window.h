#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdint.h>

#include <xcb/xcb.h>

#include "global.h"

client_t *client_create(const xcb_window_t);
void client_add(client_t *);
void client_unlink(client_t *);
bool client_kill(client_t *);
client_t *client_locate(const xcb_window_t);
client_t *handle_window(xcb_window_t);

void client_move(client_t *, const int16_t, const int16_t);
void client_resize(client_t *, const uint16_t, const uint16_t);
void client_move_resize(client_t *, const int16_t, const int16_t, const uint16_t, const uint16_t);
void client_move_resize_geom(client_t *, const xcb_rectangle_t);
void client_toggle_fullscreen(client_t *c);

bool window_update_geom(const xcb_window_t, xcb_rectangle_t *);
bool window_override_redirect(const xcb_window_t);
bool window_is_urgent(const xcb_window_t);
void window_set_border_width(const xcb_window_t, const uint16_t);

void window_show(const xcb_window_t);
void window_hide(const xcb_window_t);

#endif
