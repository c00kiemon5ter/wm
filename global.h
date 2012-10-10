#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

#define BUFLEN 256

typedef struct {
    int x, y, w, h;
} rectangle_t;

typedef struct client_t {
    rectangle_t geom;
    xcb_window_t win;
    struct client_t *next;
} client_t;

typedef struct monitor_t {
    rectangle_t geom;
    unsigned int tags;
    struct monitor_t *next;
} monitor_t;

typedef struct {
    bool running;

    char wm_name[BUFLEN];
    int default_screen;

    xcb_screen_t *screen;
    xcb_connection_t *connection;

    xcb_ewmh_connection_t *ewmh;
} global_configuration_t;

extern global_configuration_t cfg;

#endif
