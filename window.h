#ifndef WINDOW_H
#define WINDOW_H

#include <xcb/xcb.h>

#include "global.h"

#define NO_NAME     "no name"

bool window_override_redirect(xcb_window_t);
bool window_update_geom(xcb_window_t, xcb_rectangle_t *);
client_t *create_client(xcb_window_t);
void add_client(client_t *);
client_t *locate(xcb_window_t);

#endif
