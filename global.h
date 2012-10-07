#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

typedef struct {
    bool running;
    int nmonitors;

    xcb_screen_t *screen;
    xcb_connection_t *connection;
    xcb_ewmh_connection_t *ewmh;
} configuration;

extern configuration cfg;

#endif
