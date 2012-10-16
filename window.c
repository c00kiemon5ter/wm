#include <string.h>
#include <stdbool.h>

#include "window.h"
#include "helpers.h"
#include "ewmh.h"
#include "icccm.h"

void window_set_border_width(xcb_window_t win, const uint16_t border_width)
{
    const uint32_t values[] = { border_width };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
}

void window_move(const xcb_window_t win, const int16_t x, const int16_t y)
{
    const uint32_t values[] = { x, y };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_MOVE, values);
}

void window_resize(const xcb_window_t win, const uint16_t width, const uint16_t height)
{
    const uint32_t values[] = { width, height };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_RESIZE, values);
}

void window_move_resize(const xcb_window_t win, const int16_t x, const int16_t y, const uint16_t width, const uint16_t height)
{
    const uint32_t values[] = { x, y, width, height };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_MOVE_RESIZE, values);
}

void window_move_resize_geom(const xcb_window_t win, const xcb_rectangle_t geom)
{
    const uint32_t values[] = { geom.x, geom.y, geom.width, geom.height };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_MOVE_RESIZE, values);
}

void toggle_fullscreen(client_t *c)
{
    c->is_fullscrn = !c->is_fullscrn;
    xcb_atom_t values[] = { c->is_fullscrn ? cfg.ewmh->_NET_WM_STATE_FULLSCREEN : XCB_NONE };
    xcb_ewmh_set_wm_state(cfg.ewmh, c->win, LENGTH(values), values);

    if (c->is_fullscrn) {
        window_set_border_width(c->win, 0);
        window_move_resize_geom(c->win, c->mon->geom);
    }
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

client_t *create_client(const xcb_window_t win)
{
    PRINTF("creating client for window: %u\n", win);

    client_t *c = (void *)0;

    if (!(c = calloc(1, sizeof(client_t))))
        return false;

    c->win = win;
    c->mon = cfg.cur_mon;
    c->tags = cfg.cur_mon->tags;

    c->is_fullscrn = ewmh_wm_state_fullscreen(win);
    c->is_floating = icccm_is_transient(win) || ewmh_wm_type_dialog(win);
    c->is_urgent = false;

    window_update_geom(win, &c->geom);

    if (!ewmh_get_window_name(win, c->name) &&
        !icccm_get_window_name(win, c->name))
        snprintf(c->name, sizeof(c->name), "%s", NO_NAME);

    c->next = (void *)0;

    return c;
}

/**
 * add given client to the end of the clients list
 */
void add_client(client_t *c)
{
    PRINTF("adding client to end of list: %u\n", c->win);

    client_t **last = &cfg.clients;
    while (*last) last = &(*last)->next;
    *last = c;
}

/**
 * locate the client that manages the given window
 */
client_t *locate(const xcb_window_t win)
{
    for (client_t **c = &cfg.clients; *c; c = &(*c)->next)
        if ((*c)->win == win) {
            PRINTF("found client for window: %u\n", win);
            return *c;
        }

    PRINTF("no client for window: %u\n", win);
    return (void *)0;
}

