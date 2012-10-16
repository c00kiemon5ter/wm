#include <string.h>
#include <stdbool.h>

#include "window.h"
#include "helpers.h"
#include "ewmh.h"
#include "icccm.h"

bool window_update_geom(xcb_window_t win, xcb_rectangle_t *geom)
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

client_t *create_client(xcb_window_t win)
{
    PRINTF("creating client for window: %u\n", win);

    client_t *c = (void *)0;

    if (!(c = calloc(1, sizeof(client_t))))
        return false;

    c->win = win;
    c->mon = cfg.cur_mon;
    c->tags = cfg.cur_mon->tags;

    c->is_fullscrn = ewmh_wm_state_fullscreen(win);
    c->is_floating = icccm_is_transient(win);
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
client_t *locate(xcb_window_t win)
{
    for (client_t **c = &cfg.clients; *c; c = &(*c)->next)
        if ((*c)->win == win) {
            PRINTF("found client for window: %u\n", win);
            return *c;
        }

    PRINTF("no client for window: %u\n", win);
    return (void *)0;
}

