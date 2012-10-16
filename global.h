#ifndef GLOBAL_H
#define GLOBAL_H

#include <limits.h>
#include <stdbool.h>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

/* mask that defines the events the wm is notified of */
#define ROOT_EVENT_MASK (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY)

/* length for small buffers */
#define BUF_NAME_LEN 256

/**
 * enumeration of tiling methods
 */
typedef enum {
    VSTACK,
    HSTACK,
    GRID,
    MONOCLE,
    FLOAT,
} layout_t;

/**
 * A monitor
 *
 * geom - the monitor geometry - x, y, width, height
 * tags - bitmask with set bits the active tags on the monitor
 * mode - the layout for this monitor
 * next - the next available monitor
 */
typedef struct monitor_t {
    xcb_rectangle_t geom;
    unsigned int tags;
    layout_t mode;
    struct monitor_t *next;
} monitor_t;

/**
 * A client
 *
 * geom - the client geometry - x, y, width, height
 * tags - a bitmask with set bits the tags
 * name - the name of the window
 * next - the next client
 * win  - the window to manage
 * mon  - the monitor to which the window belongs
 *
 * is_urgent    - set if the client has set an urgent hint
 * is_floating  - set if the client is floating
 * is_fullscrn  - set if the client is fullscreen
 */
typedef struct client_t {
    xcb_rectangle_t geom;
    unsigned int tags;
    char name[BUF_NAME_LEN];
    bool is_urgent, is_floating, is_fullscrn;
    xcb_window_t win;
    struct client_t *next;
    monitor_t *mon;
} client_t;

/**
 * global shared variables
 *
 * running      - state of the window manager
 * def_screen   - the screen number
 * screen       - the screen struct
 * conx         - the connection to the X server
 * ewmh         - connection to manage ewmh
 * tag_names    - array with tag names
 * monitors     - list of available monitors
 * clients      - list of all managed clients
 * cur_client   - the current, active and focused client
 * cur_mon      - the current, active and focused monitor
 */
struct configuration {
    bool running;
    int def_screen;
    xcb_screen_t *screen;
    xcb_connection_t *conn;
    xcb_ewmh_connection_t *ewmh;
    char tag_names[sizeof(unsigned int) * CHAR_BIT][BUF_NAME_LEN];
    monitor_t *monitors;
    client_t *clients;
    client_t *cur_client;
    monitor_t *cur_mon;
};

extern struct configuration cfg;

#endif
