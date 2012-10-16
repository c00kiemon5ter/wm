#ifndef WINDOW_H
#define WINDOW_H

#include <xcb/xcb.h>

#include "global.h"

#define NO_NAME     "no name"

client_t *create_client(xcb_window_t);
void add_client(client_t *);
client_t *locate(xcb_window_t);

#endif
