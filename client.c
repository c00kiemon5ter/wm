
#include "client.h"
#include "helpers.h"
#include "monitor.h"
#include "rules.h"
#include "window.h"
#include "ewmh.h"
#include "icccm.h"

#define WINDOW_NO_NAME      "no name"

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
    c->mon = cfg.monitors;

    BITMASK_SET(c->tags, cfg.monitors->tags);

    c->is_fullscrn = ewmh_wm_state_fullscreen(win);
    c->is_floating = icccm_is_transient(win) || ewmh_wm_type_dialog(win);
    c->is_urgent = false;

    window_update_geom(win, &c->geom);

    if (!icccm_get_window_class(win, c->class, c->instance))
        warn("failed to get class and instance name for client: %u\n", win);

    if (!ewmh_get_window_title(win, c->title) &&
        !icccm_get_window_title(win, c->title))
        snprintf(c->title, sizeof(c->title), "%s", WINDOW_NO_NAME);

    c->vnext = (void *)0;
    c->fnext = (void *)0;

    return c;
}

/**
 * add given client to head of clients visual list
 */
void client_link_head(client_t *c)
{
    PRINTF("linking client to head of vlist: %u\n", c->win);

    c->vnext = cfg.vlist;
    cfg.vlist = c;
}

/**
 * add given client to end of clients visual list
 */
void client_link_tail(client_t *c)
{
    PRINTF("linking client to end of vlist: %u\n", c->win);

    client_t **last = &cfg.vlist;
    while (*last) last = &(*last)->vnext;
    *last = c;
}

/**
 * add given client to head of clients focus list
 */
void client_flink(client_t *c)
{
    PRINTF("linking client to focus list: %u\n", c->win);

    c->fnext = cfg.flist;
    cfg.flist = c;
}

/**
 * remove given client from the clients visual list
 */
void client_vunlink(client_t *c)
{
    PRINTF("unlinking client from visual list: %u\n", c->win);

    client_t **p = &cfg.vlist;
    while (*p && *p != c) p = &(*p)->vnext;
    *p = c->vnext;
    c->vnext = (void *)0;

    PRINTF("vp is now: %u\n", (*p) ? (*p)->win : 0);
}

/**
 * remove given client from the clients focus list
 */
void client_funlink(client_t *c)
{
    PRINTF("unlinking client from focus list: %u\n", c->win);

    client_t **p = &cfg.flist;
    while (*p && *p != c) p = &(*p)->fnext;
    *p = c->fnext;
    c->fnext = (void *)0;

    PRINTF("fp is now: %u\n", (*p) ? (*p)->win : 0);
}

/**
 * remove given client from the clients list
 */
inline
void client_unlink(client_t *c)
{
    PRINTF("unlinking client from client list: %u\n", c->win);

    client_vunlink(c);
    client_funlink(c);
}

/**
 * focus the given client
 * make the clients monitor the current monitor
 */
void client_focus(client_t *c)
{
    PRINTF("focusing client: %u\n", c ? c->win : 0);

    ewmh_update_active_window(c ? c->win : 0);

    if (!c || cfg.flist == c)
        return;

    client_funlink(c);
    client_flink(c);
    monitor_focus(c->mon);

    client_update_border(c);
    xcb_set_input_focus(cfg.conn, XCB_INPUT_FOCUS_POINTER_ROOT, c->win, XCB_CURRENT_TIME);

    PRINTF("focused client is: %u\n", c->win);
}

void client_focus_next(void)
{
    if (!cfg.flist)
        return;

    client_t *next = cfg.flist->vnext;
    while (next && !(IS_VISIBLE(next) && ON_MONITOR(cfg.monitors, next)))
            next = next->vnext;

    if (!next)
        for (next = cfg.vlist; next != cfg.flist && !(IS_VISIBLE(next) && ON_MONITOR(cfg.monitors, next)); next = next->vnext);

    client_focus(next);
}

void client_focus_prev(void)
{
    if (!cfg.flist)
        return;

    client_t *p = (void *)0;
    client_t *v = cfg.vlist;

    for (; v && v != cfg.flist; v = v->vnext)
        if (IS_VISIBLE(v) && ON_MONITOR(cfg.monitors, v))
            p = v;

    if (!p)
        for (v = v->vnext; v; v = v->vnext)
            if (IS_VISIBLE(v) && ON_MONITOR(cfg.monitors, v))
                p = v;

    client_focus(p);
}

void client_focus_first(const monitor_t *m)
{
    client_t *c = cfg.flist;
    while (c && !(IS_VISIBLE(c) && ON_MONITOR(m, c)))
        c = c->fnext;

    client_focus(c);
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

    if (icccm_close_window(c->win))
        return true;

    xcb_kill_client(cfg.conn, c->win);
    client_remove(c);

    return false;
}

/**
 * remove the given client
 */
void client_remove(client_t *c)
{
    PRINTF("removing client: %u\n", c->win);

    client_focus(c->fnext);
    client_unlink(c);
    free(c);
}

/**
 * locate the client that manages the given window
 */
client_t *client_locate(const xcb_window_t win)
{
    for (client_t **c = &cfg.vlist; *c; c = &(*c)->vnext)
        if ((*c)->win == win) {
            PRINTF("found client for window: %u\n", win);
            return *c;
        }

    PRINTF("no client for window: %u\n", win);
    return (void *)0;
}

/**
 * create a new client for the given window
 * apply rules and add to client list
 */
client_t *handle_window(const xcb_window_t win)
{
    if (client_locate(win) || window_override_redirect(win) || ewmh_wm_type_ignored(win))
        return (void *)0;

    client_t *c = client_create(win);
    if (!c)
        err("failed to allocate client for window: %u\n", win);

    PRINTF("client window  : %u\n", c->win);
    PRINTF("client class   : %s\n", c->class);
    PRINTF("client instance: %s\n", c->instance);
    PRINTF("client title   : %s\n", c->title);
    PRINTF("client floating: %s\n", BOOLSTR(c->is_floating));
    PRINTF("client fullscrn: %s\n", BOOLSTR(c->is_fullscrn));
    PRINTF("client tagmask : %x\n", c->tags);
    PRINTF("client geom x  : %u\n", c->geom.x);
    PRINTF("client geom y  : %u\n", c->geom.y);
    PRINTF("client geom w  : %u\n", c->geom.width);
    PRINTF("client geom h  : %u\n", c->geom.height);

    const uint32_t values[] = { XCB_EVENT_MASK_PROPERTY_CHANGE };
    xcb_change_window_attributes(cfg.conn, win, XCB_CW_EVENT_MASK, values);

    rules_apply(c);
    client_link_tail(c);

    return c;
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

inline
void client_update_geom(client_t *c)
{
    window_update_geom(c->win, &c->geom);
}

inline
void client_update_border(const client_t *c)
{
    window_set_border_width(c->win, c->mon->border);
}

inline
void client_hide(const client_t *c)
{
    window_hide(c->win);
}

inline
void client_show(const client_t *c)
{
    window_show(c->win);
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

