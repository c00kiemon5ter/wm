#ifndef GLOBAL_H
#define GLOBAL_H

#include <limits.h>
#include <stdbool.h>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

/* mask that defines the events the wm is notified of */
#define ROOT_EVENT_MASK (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY)

/* length for small buffers */
#define BUFLEN 256

/**
 * A client
 *
 * geom - the client geometry - x, y, width, height
 * tags - a bitmask with set bits the tags
 * name - the name of the window
 * win  - the window to manage
 * next - the next client
 *
 * is_urgent    - set if the client has set an urgent hint
 * is_floating  - set if the client is floating
 * is_fullscrn  - set if the client is fullscreen
 */
typedef struct client_t {
    xcb_rectangle_t geom;
    unsigned int tags;
    char name[BUFLEN];
    bool is_urgent, is_floating, is_fullscrn;
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
    xcb_rectangle_t geom;
    unsigned int tags;
    struct monitor_t *next;
} monitor_t;

/**
 * global shared variables
 *
 * running      - state of the window manager
 * wm_name      - name of the window manager
 * def_screen   - the screen number
 * screen       - the screen struct
 * conx         - the connection to the X server
 * ewmh         - connection to manage ewmh
 * tag_names    - array with tag names
 * monitors     - list of available monitors
 * clients      - list of all managed clients
 * cur_client   - the current active and focused client
 */
struct configuration {
    bool running;
    char wm_name[BUFLEN];
    int def_screen;
    xcb_screen_t *screen;
    xcb_connection_t *conn;
    xcb_ewmh_connection_t *ewmh;
    char tag_names[sizeof(unsigned int) * CHAR_BIT][BUFLEN];
    monitor_t *monitors;
    client_t *clients;
    client_t *cur_client;
};

extern struct configuration cfg;

#endif
