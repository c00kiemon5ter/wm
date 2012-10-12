#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

/* length for small buffers */
#define BUFLEN 256

/**
 * A rectangle
 *
 * x - the position on the x axis
 * y - the position on the y axis
 * w - the width of the rectangle
 * h - the height of the rectangle
 */
typedef struct {
    int16_t x, y;
    uint16_t w, h;
} rectangle_t;

/**
 * A client
 *
 * geom - the client geometry - x, y, width, height
 * tags - a bitmask with set bits the tags
 * name - the name of the window
 * win  - the window to manage
 * next - the next client
 *
 * is_floating - set if the client is floating
 * is_urgent   - set if the client has set an urgent hint
 * is_fullscrn - set if the client is fullscreen
 */
typedef struct client_t {
    rectangle_t geom;
    unsigned int tags;
    char name[BUFLEN];
    bool is_floating, is_urgent, is_fullscrn;
    xcb_window_t win;
    struct client_t *next;
} client_t;

/**
 * A monitor
 *
 * geom - the monitor geometry - x, y, width, height
 * tags - bitmask with set bits the active tags on the monitor
 * next - the next available monitor
 */
typedef struct monitor_t {
    rectangle_t geom;
    unsigned int tags;
    struct monitor_t *next;
} monitor_t;

/**
 * A tag
 *
 * nbit - the position on the bitmask
 * name - the tag name/label
 * next - the next available tag
 */
typedef struct tag_t {
    unsigned int bit;
    char name[BUFLEN];
    struct tag_t *next;
} tag_t;

/* global shared variables */
typedef struct {
    /* state of the window manager */
    bool running;
    /* name of the window manager */
    char wm_name[BUFLEN];
    /* the screen number and struct */
    int default_screen;
    xcb_screen_t *screen;
    /* the connection to the X server */
    xcb_connection_t *connection;
    /* connection to manage ewmh */
    xcb_ewmh_connection_t *ewmh;
    /* list of available monitors */
    monitor_t *monitors;
    /* list of all managed clients */
    client_t *clients;
    /* the current active and focused client */
    client_t *current_client;
} global_configuration_t;

extern global_configuration_t cfg;

#endif
