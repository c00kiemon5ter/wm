
#include "client.h"
#include "helpers.h"
#include "monitor.h"
#include "rules.h"
#include "ewmh.h"
#include "icccm.h"

#define CONFIG_WINDOW_MOVE              XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#define CONFIG_WINDOW_RESIZE            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define CONFIG_WINDOW_MOVE_RESIZE       CONFIG_WINDOW_MOVE | CONFIG_WINDOW_RESIZE
#define WINDOW_NO_NAME                  "no name"

typedef enum {
    LIST_VISUAL,
    LIST_FOCUS,
    LIST_TYPES
} list_t;

static
bool window_override_redirect(const xcb_window_t win)
{
    const xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(cfg.conn, win);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    const bool state = reply->override_redirect;

    free(reply);

    return state;
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
    c->mon = cfg.monitors;

    BITMASK_SET(c->tags, cfg.monitors->tags);

    c->is_fullscrn = ewmh_wm_state_fullscreen(win);
    c->is_floating = icccm_is_transient(win) || ewmh_wm_type_dialog(win);
    c->is_urgent = false;

    client_update_geom(c);

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
 * set the client's monitor the current monitor
 */
void client_focus(client_t *c)
{
    xcb_window_t w = c ? c->win : XCB_NONE;

    PRINTF("focusing client: %u\n", w);

    ewmh_update_active_window(w);

    if (c && cfg.flist != c) {
        client_funlink(c);
        client_flink(c);
        monitor_focus(c->mon);
        client_update_border(c, c->mon->border);
    }

    xcb_set_input_focus(cfg.conn, XCB_INPUT_FOCUS_POINTER_ROOT, w, XCB_CURRENT_TIME);

    PRINTF("focused client is: %u\n", w);
}

/**
 * return the next client on the given list
 * that is visible and on the given monitor
 */
static
client_t *client_next(const client_t *c, const monitor_t *m, const list_t list)
{
    if (!c || !m || list >= LIST_TYPES)
        return (void *)0;

    client_t *next = (list == LIST_FOCUS) ? c->fnext : (list == LIST_VISUAL) ? c->vnext : (void *)0;
    while (next && !(IS_VISIBLE(next) && ON_MONITOR(m, next)))
        next = (list == LIST_FOCUS) ? next->fnext : (list == LIST_VISUAL) ? next->vnext : (void *)0;

    if (!next)
        for (next = (list == LIST_FOCUS) ? cfg.flist : (list == LIST_VISUAL) ? cfg.vlist : (void *)0;
             next != c && !(IS_VISIBLE(next) && ON_MONITOR(m, next));
             next = (list == LIST_FOCUS) ? next->fnext : (list == LIST_VISUAL) ? next->vnext : (void *)0);

    return next;
}

/**
 * return the next visible client from the given on the given monitor
 */
inline
client_t *client_vnext(const client_t *c, const monitor_t *m)
{
    return client_next(c, m, LIST_VISUAL);
}

/**
 * return the previously focused client from the given on the given monitor
 */
inline
client_t *client_fnext(const client_t *c, const monitor_t *m)
{
    return client_next(c, m, LIST_FOCUS);
}

/**
 * return the previous client on the given list
 * that is visible and on the given monitor
 */
static
client_t *client_prev(const client_t *c, const monitor_t *m, const list_t list)
{
    if (!c || !m || list >= LIST_TYPES)
        return (void *)0;

    client_t *p = (void *)0;
    client_t *e = (list == LIST_FOCUS) ? cfg.flist : (list == LIST_VISUAL) ? cfg.vlist : (void *)0;

    for (; e && e != c; e = (list == LIST_FOCUS) ? e->fnext : (list == LIST_VISUAL) ? e->vnext : (void *)0)
        if (IS_VISIBLE(e) && ON_MONITOR(m, e))
            p = e;

    if (!p)
        for (e = (list == LIST_FOCUS) ? c->fnext : (list == LIST_VISUAL) ? c->vnext : (void *)0; e;
             e = (list == LIST_FOCUS) ? e->fnext : (list == LIST_VISUAL) ? e->vnext : (void *)0)
            if (IS_VISIBLE(e) && ON_MONITOR(m, e))
                p = e;

    return p;
}

/**
 * return the previous visual client from the given on the given monitor
 */
inline
client_t *client_vprev(const client_t *c, const monitor_t *m)
{
    return client_prev(c, m, LIST_VISUAL);
}

/**
 * return the oldest focused client from the given on the given monitor
 */
inline
client_t *client_fprev(const client_t *c, const monitor_t *m)
{
    return client_prev(c, m, LIST_FOCUS);
}

/**
 * move the given client (c)
 * after the given client (t)
 */
inline
void client_move_after(client_t *c, client_t *t)
{
    client_vunlink(c);
    c->vnext = t->vnext;
    t->vnext = c;
}

/**
 * move the given client (c)
 * before the given client (t)
 *
 * FIXME this is buggy ofcourse
 */
inline
void client_move_before(client_t *c, client_t *t)
{
    client_move_after(t, c);
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

    if (IS_VISIBLE(c) && ON_MONITOR(cfg.monitors, c)) {
        client_t *f = client_fnext(c, c->mon);
        client_focus(f);
    }

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

void client_move(client_t *c, const int16_t x, const int16_t y)
{
    const uint32_t values[] = { c->geom.x = x, c->geom.y = y };
    xcb_configure_window(cfg.conn, c->win, CONFIG_WINDOW_MOVE, values);
}

void client_resize(client_t *c, const uint16_t w, const uint16_t h)
{
    const uint32_t values[] = { c->geom.width = w, c->geom.height = h };
    xcb_configure_window(cfg.conn, c->win, CONFIG_WINDOW_RESIZE, values);
}

void client_move_resize(client_t *c, const int16_t x, const int16_t y, const uint16_t w, const uint16_t h)
{
    const uint32_t values[] = { c->geom.x = x, c->geom.y = y, c->geom.width = w, c->geom.height = h };
    xcb_configure_window(cfg.conn, c->win, CONFIG_WINDOW_MOVE_RESIZE, values);
}

inline
void client_move_resize_geom(client_t *c, const xcb_rectangle_t geom)
{
    client_move_resize(c, geom.x, geom.y, geom.width, geom.height);
}

bool client_update_geom(client_t *c)
{
    const xcb_get_geometry_cookie_t cookie = xcb_get_geometry(cfg.conn, c->win);
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    c->geom.x      = reply->x;
    c->geom.y      = reply->y;
    c->geom.width  = reply->width;
    c->geom.height = reply->height;

    free(reply);

    return true;
}

void client_update_border(const client_t *c, const uint32_t border_width)
{
    const uint32_t values[] = { border_width };
    xcb_configure_window(cfg.conn, c->win, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
}

inline
void client_update_urgency(client_t *c)
{
    c->is_urgent = icccm_has_urgent_hint(c->win);
}

/* ** visibility functions ** */

static
void window_set_visibility(const xcb_window_t win, bool visible)
{
    uint32_t values_off[] = { ROOT_EVENT_MASK & ~XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
    uint32_t values_on[]  = { ROOT_EVENT_MASK };

    xcb_grab_server(cfg.conn);

    xcb_change_window_attributes(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values_off);
    if (visible)
        xcb_map_window(cfg.conn, win);
    else
        xcb_unmap_window(cfg.conn, win);
    xcb_change_window_attributes(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values_on);

    xcb_ungrab_server(cfg.conn);
}

inline
void client_show(const client_t* c)
{
    window_set_visibility(c->win, true);
}

inline
void client_hide(const client_t* c)
{
    window_set_visibility(c->win, false);
}

void client_toggle_fullscreen(client_t *c)
{
    c->is_fullscrn = !c->is_fullscrn;
    xcb_atom_t values[] = { c->is_fullscrn ? cfg.ewmh->_NET_WM_STATE_FULLSCREEN : XCB_NONE };
    xcb_ewmh_set_wm_state(cfg.ewmh, c->win, LENGTH(values), values);

    if (c->is_fullscrn) {
        client_update_border(c, 0);
        client_move_resize_geom(c, c->mon->geom);
    }
}

/* ** stacking order functions ** */

void client_raise(const client_t *c)
{
    const uint32_t values[] = { XCB_STACK_MODE_ABOVE };
    xcb_configure_window(cfg.conn, c->win, XCB_CONFIG_WINDOW_STACK_MODE, values);
}

void client_lower(const client_t *c)
{
    const uint32_t values[] = { XCB_STACK_MODE_BELOW };
    xcb_configure_window(cfg.conn, c->win, XCB_CONFIG_WINDOW_STACK_MODE, values);
}

