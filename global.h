#ifndef GLOBAL_H
#define GLOBAL_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

/* mask that defines the events the wm is notified of */
#define ROOT_EVENT_MASK     ( XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT \
                            | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY   \
                            | XCB_EVENT_MASK_PROPERTY_CHANGE       )

/* length for small buffers */
#define BUF_NAME_LEN 256

/* default master area ratio */
#define M_AREA_FACT 0.55

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
 * a monitor
 *
 * geom   - the monitor geometry - x, y, width, height
 * tags   - bitmask with set bits the active tags on the monitor
 * mode   - the layout for this monitor
 * m_area - adjustment of the master area
 * m_wins - number of windows in master area
 * next   - the next available monitor
 */
typedef struct monitor_t {
    xcb_rectangle_t geom;
    uint16_t tags;
    uint16_t m_area;
    uint16_t m_wins;
    layout_t mode;
    struct monitor_t *next;
} monitor_t;

/**
 * a client
 *
 * geom - the client geometry - x, y, width, height
 * tags - a bitmask with set bits the tags of the client
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
    uint16_t tags;
    char title[BUF_NAME_LEN], class[BUF_NAME_LEN], instance[BUF_NAME_LEN];
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
    char tag_names[sizeof(uint16_t) * CHAR_BIT][BUF_NAME_LEN];
    monitor_t *monitors;
    client_t *clients;
    client_t *client_cur;
    monitor_t *monitor_cur;
};

extern struct configuration cfg;

#endif
