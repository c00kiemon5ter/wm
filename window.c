#include <stdbool.h>

#include "window.h"
#include "helpers.h"

bool window_is_transient(xcb_window_t win)
{
    return false;
}

bool window_is_fullscreen(xcb_window_t win)
{
    return false;
}

char *window_name(xcb_window_t win)
{
    // xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_name_unchecked(cfg.ewmh, win);

    // xcb_ewmh_get_utf8_strings_reply_t *data = NULL;
    // xcb_ewmh_get_wm_name_reply(cfg.ewmh, cookie, data, (void *)0);

    // xcb_get_property_reply_t *reply = NULL;

    // if (!data)
    //     err("data is NULL\n", "");
    // return data->strings;
    return NULL;
}

client_t *create_client(xcb_window_t win)
{
    PRINTF("creating client for window: %u\n", win);

    client_t *c = NULL;

    if (!(c = calloc(1, sizeof(client_t))))
        return false;

    c->win = win;
    c->mon = cfg.cur_mon;
    c->tags = cfg.cur_mon->tags;

    c->is_fullscrn = window_is_fullscreen(win);
    c->is_floating = window_is_transient(win);
    c->is_urgent = false;

    /* FIXME get window attributs */
    c->geom = (const xcb_rectangle_t){ .x = 0, .y = 0, .width = 200, .height = 100 };
    /* FIXME get window name */
    char *name = window_name(win);
    if (!name)
        name = NO_NAME;
    snprintf(c->name, sizeof(c->name), "%s", name);

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
    return NULL;
}

