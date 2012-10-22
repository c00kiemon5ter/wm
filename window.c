#include <string.h>

#include "window.h"
#include "helpers.h"
#include "ewmh.h"
#include "icccm.h"

#define WINDOW_NO_NAME                  "no name"
#define CONFIG_WINDOW_MOVE              XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#define CONFIG_WINDOW_RESIZE            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define CONFIG_WINDOW_MOVE_RESIZE       CONFIG_WINDOW_MOVE | CONFIG_WINDOW_RESIZE
#define POINTER_GRAB_MASK               ( XCB_EVENT_MASK_BUTTON_PRESS   \
                                        | XCB_EVENT_MASK_BUTTON_RELEASE \
                                        | XCB_EVENT_MASK_POINTER_MOTION )

/* ** window move and resize function ** */

void window_move(const xcb_window_t win, const int16_t x, const int16_t y)
{
    const uint32_t values[] = { x, y };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_MOVE, values);
}

void window_resize(const xcb_window_t win, const uint16_t width, const uint16_t height)
{
    const uint32_t values[] = { width, height };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_RESIZE, values);
}

void window_move_resize(const xcb_window_t win, const int16_t x, const int16_t y, const uint16_t width, const uint16_t height)
{
    const uint32_t values[] = { x, y, width, height };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_MOVE_RESIZE, values);
}

void window_move_resize_geom(const xcb_window_t win, const xcb_rectangle_t geom)
{
    const uint32_t values[] = { geom.x, geom.y, geom.width, geom.height };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_MOVE_RESIZE, values);
}

/* ** visibility functions ** */

void window_set_visibility(const xcb_window_t win, bool visible)
{
    uint32_t values_off[] = { ROOT_EVENT_MASK & ~XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
    uint32_t values_on[]  = { ROOT_EVENT_MASK };
    xcb_change_window_attributes(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values_off);
    if (visible)
        xcb_map_window(cfg.conn, win);
    else
        xcb_unmap_window(cfg.conn, win);
    xcb_change_window_attributes(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values_on);
}

inline
void window_show(const xcb_window_t win)
{
    window_set_visibility(win, true);
}

inline
void window_hide(const xcb_window_t win)
{
    window_set_visibility(win, false);
}

/**
 * initialize allocate and return the new client
 */
client_t *client_create(const xcb_window_t win)
{
    PRINTF("creating client for window: %u\n", win);

    client_t *c = calloc(1, sizeof(client_t));

    if (!c)
        return false;

    c->win = win;
    c->mon = cfg.cur_mon;

    BITMASK_SET(c->tags, cfg.cur_mon->tags);

    c->is_fullscrn = ewmh_wm_state_fullscreen(win);
    c->is_floating = icccm_is_transient(win) || ewmh_wm_type_dialog(win);
    c->is_urgent = false;

    window_update_geom(win, &c->geom);

    if (!ewmh_get_window_name(win, c->name) &&
        !icccm_get_window_name(win, c->name))
        snprintf(c->name, sizeof(c->name), "%s", WINDOW_NO_NAME);

    c->next = (void *)0;

    return c;
}

/**
 * add given client to the end of the clients list
 */
void client_add(client_t *c)
{
    PRINTF("adding client to end of list: %u\n", c->win);

    client_t **last = &cfg.clients;
    while (*last) last = &(*last)->next;
    *last = c;
}

/**
 * remove given client from the clients list
 */
void client_unlink(client_t *c)
{
    PRINTF("unlinking client from client list: %u\n", c->win);

    client_t **p = &cfg.clients;
    for (; *p && *p != c; p = &(*p)->next);

    if (!*p) return;

    *p = c->next;
    c->next = (void *)0;

    PRINTF("p is now: %u\n", (*p) ? (*p)->win : 0);
}

/**
 * close the given client's window
 * or kill it if it won't close
 *
 * a 'true' return value means that the window will
 * close gracefully on the next event processing.
 * a 'false' return value means that the window
 * was killed and further actions maybe required,
 * like, manually calling 'tile()' to arrange the
 * new list of windows.
 */
bool client_kill(client_t *c)
{
    PRINTF("killing client: %u\n", c->win);

    bool state = icccm_close_window(c->win);

    if (!state)
        xcb_kill_client(cfg.conn, c->win);

    xcb_flush(cfg.conn);

    return state;
}

/**
 * locate the client that manages the given window
 */
client_t *client_locate(const xcb_window_t win)
{
    for (client_t **c = &cfg.clients; *c; c = &(*c)->next)
        if ((*c)->win == win) {
            PRINTF("found client for window: %u\n", win);
            return *c;
        }

    PRINTF("no client for window: %u\n", win);
    return (void *)0;
}

/* ** client move and resize functions ** */

inline
void client_move(client_t *c, const int16_t x, const int16_t y)
{
    window_move(c->win, c->geom.x = x, c->geom.y = y);
}

inline
void client_resize(client_t *c, const uint16_t w, const uint16_t h)
{
    window_resize(c->win, c->geom.width = w, c->geom.height = h);
}

inline
void client_move_resize(client_t *c, const int16_t x, const int16_t y, const uint16_t w, const uint16_t h)
{
    window_move_resize(c->win, c->geom.x = x, c->geom.y = y, c->geom.width = w, c->geom.height = h);
}

inline
void client_move_resize_geom(client_t *c, const xcb_rectangle_t geom)
{
    window_move_resize_geom(c->win, c->geom = geom);
}

void client_toggle_fullscreen(client_t *c)
{
    c->is_fullscrn = !c->is_fullscrn;
    xcb_atom_t values[] = { c->is_fullscrn ? cfg.ewmh->_NET_WM_STATE_FULLSCREEN : XCB_NONE };
    xcb_ewmh_set_wm_state(cfg.ewmh, c->win, LENGTH(values), values);

    if (c->is_fullscrn) {
        window_set_border_width(c->win, 0);
        window_move_resize_geom(c->win, c->mon->geom);
    }
}

bool window_update_geom(const xcb_window_t win, xcb_rectangle_t *geom)
{
    const xcb_get_geometry_cookie_t cookie = xcb_get_geometry(cfg.conn, win);
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    geom->x = reply->x;
    geom->y = reply->y;
    geom->width = reply->width;
    geom->height = reply->height;

    free(reply);

    return true;
}

bool window_override_redirect(const xcb_window_t win)
{
    bool state = false;

    const xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(cfg.conn, win);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return state;

    state = reply->override_redirect;

    free(reply);

    return state;
}

inline
bool window_is_urgent(const xcb_window_t win)
{
    return icccm_has_urgent_hint(win);
}

void window_set_border_width(xcb_window_t win, const uint16_t border_width)
{
    const uint32_t values[] = { border_width };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
}

/* ** pointer related functions ** */

bool window_grab_pointer(void)
{
    xcb_grab_pointer_cookie_t cookie = xcb_grab_pointer_unchecked(cfg.conn, false, cfg.screen->root, POINTER_GRAB_MASK,
                                        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);
    xcb_grab_pointer_reply_t *reply = xcb_grab_pointer_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    free(reply);
    return true;
}

inline
void window_ungrab_pointer(void)
{
    xcb_ungrab_pointer(cfg.conn, XCB_CURRENT_TIME);
}

void window_set_pointer(const xcb_window_t win, const uint16_t pointer_id)
{
    xcb_font_t font = xcb_generate_id(cfg.conn);
    xcb_open_font(cfg.conn, font, sizeof("cursor"), "cursor");

    xcb_cursor_t cursor = xcb_generate_id(cfg.conn);
    xcb_create_glyph_cursor(cfg.conn, cursor, font, font, pointer_id, pointer_id + 1, 0xFFFF,0xFFFF,0xFFFF, 0,0,0);

    xcb_gcontext_t gc = xcb_generate_id(cfg.conn);
    xcb_create_gc(cfg.conn, gc, win, XCB_GC_FONT, &font);

    xcb_change_window_attributes (cfg.conn, win, XCB_CW_CURSOR, &cursor);

    xcb_free_cursor(cfg.conn, cursor);
    xcb_close_font(cfg.conn, font);
}

