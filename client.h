#ifndef CLIENT_H
#define CLIENT_H

#include <stdint.h>
#include <stdbool.h>

#include <xcb/xcb.h>

#include "global.h"

#define ON_MONITOR(m,c) (m == c->mon)
#define IS_VISIBLE(c)   (BITMASK_CHECK(c->mon->tags, c->tags))
#define IS_TILED(c)     (!c->is_floating && !c->is_fullscrn)

client_t *client_create(const xcb_window_t);

void client_link_head(client_t *);
void client_link_tail(client_t *);
void client_flink(client_t *);

void client_vunlink(client_t *);
void client_funlink(client_t *);
void client_unlink(client_t *);

void client_focus(client_t *);

client_t *client_fnext(const client_t *, const monitor_t *);
client_t *client_fprev(const client_t *, const monitor_t *);
client_t *client_vnext(const client_t *, const monitor_t *);
client_t *client_vprev(const client_t *, const monitor_t *);

void client_move_after(client_t *, client_t *);
void client_move_before(client_t *, client_t *);

bool client_kill(client_t *);
void client_remove(client_t *);

client_t *client_locate(const xcb_window_t);
client_t *handle_window(const xcb_window_t);

void client_move(client_t *, const int16_t, const int16_t);
void client_resize(client_t *, const uint16_t, const uint16_t);
void client_move_resize(client_t *, const int16_t, const int16_t, const uint16_t, const uint16_t);
void client_move_resize_geom(client_t *, const xcb_rectangle_t);

void client_update_geom(client_t *);
void client_update_border(const client_t *);
void client_update_urgency(client_t *);
void client_toggle_fullscreen(client_t *);

void client_hide(const client_t *);
void client_show(const client_t *);

void client_raise(const client_t *);
void client_lower(const client_t *);

#endif
