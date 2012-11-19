#include <stdint.h>

#include "events.h"
#include "wm.h"
#include "helpers.h"
#include "client.h"
#include "pointer.h"
#include "ewmh.h"
#include "tile.h"

void configure_request(xcb_generic_event_t *evt)
{
    xcb_configure_request_event_t *e = (xcb_configure_request_event_t *)evt;

    PRINTF("configure request %u\n", e->window);

    client_t *c = client_locate(e->window);
    if (c && !c->is_floating) {
        const xcb_configure_notify_event_t evt = {
            .response_type = XCB_CONFIGURE_NOTIFY,
            .event         = e->window,
            .window        = e->window,
            .above_sibling = XCB_NONE,
            .x             = c->geom.x,
            .y             = c->geom.y,
            .width         = c->geom.width,
            .height        = c->geom.height,
            .border_width  = c->mon->border,
            .override_redirect = false
        };
        xcb_send_event(cfg.conn, false, e->window, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&evt);
    } else {
        uint32_t values[7] = { 0 };
        uint32_t *val = values;

        if (BITMASK_CHECK(e->value_mask, XCB_CONFIG_WINDOW_X))
            *val++ = e->x;
        if (BITMASK_CHECK(e->value_mask, XCB_CONFIG_WINDOW_Y))
            *val++ = e->y;
        if (BITMASK_CHECK(e->value_mask, XCB_CONFIG_WINDOW_WIDTH))
            *val++ = e->width;
        if (BITMASK_CHECK(e->value_mask, XCB_CONFIG_WINDOW_HEIGHT))
            *val++ = e->height;
        if (BITMASK_CHECK(e->value_mask, XCB_CONFIG_WINDOW_BORDER_WIDTH))
            *val++ = e->border_width;
        if (BITMASK_CHECK(e->value_mask, XCB_CONFIG_WINDOW_SIBLING))
            *val++ = e->sibling;
        if (BITMASK_CHECK(e->value_mask, XCB_CONFIG_WINDOW_STACK_MODE))
            *val++ = e->stack_mode;

        xcb_configure_window(cfg.conn, e->window, e->value_mask, values);
        if (c)
            client_update_geom(c);
    }
}

void map_request(xcb_generic_event_t *evt)
{
    const xcb_map_request_event_t *e = (xcb_map_request_event_t *)evt;

    PRINTF("map request %u\n", e->window);

    client_t *c = handle_window(e->window);
    if (!c || !IS_VISIBLE(c) || !ON_MONITOR(cfg.monitors, c))
        return;

    client_show(c);
    tile(c->mon);
    client_focus(c);
}

void client_message(xcb_generic_event_t *evt)
{
    xcb_client_message_event_t *e = (xcb_client_message_event_t *)evt;

    PRINTF("client message %u\n", e->window);

    client_t *c = client_locate(e->window);
    if (!c)
        return;

    if (e->type == cfg.ewmh->_NET_WM_STATE
            && ((e->data.data32[1] == cfg.ewmh->_NET_WM_STATE_FULLSCREEN) ||
                (e->data.data32[2] == cfg.ewmh->_NET_WM_STATE_FULLSCREEN))
            && ((e->data.data32[0] == XCB_EWMH_WM_STATE_ADD    && !c->is_fullscrn) ||
                (e->data.data32[0] == XCB_EWMH_WM_STATE_REMOVE &&  c->is_fullscrn) ||
                (e->data.data32[0] == XCB_EWMH_WM_STATE_TOGGLE))) {
        PRINTF("state fullscreen for client: %u\n", e->window);
        client_toggle_fullscreen(c);
        tile(c->mon);
    } else if (e->type == cfg.ewmh->_NET_ACTIVE_WINDOW) {
        PRINTF("activating client: %u\n", e->window);
        if (IS_VISIBLE(c) && ON_MONITOR(cfg.monitors, c)) {
            ewmh_update_active_window(c->win);
            client_focus(c);
        }
    }
}

void property_notify(xcb_generic_event_t *evt)
{
    xcb_property_notify_event_t *e = (xcb_property_notify_event_t *)evt;

    PRINTF("property notify: %u\n", e->window);

    if (e->atom != XCB_ATOM_WM_HINTS || e->window == cfg.screen->root)
        return;

    client_t *c = client_locate(e->window);
    if (c)
        client_update_urgency(c);
}

static
void event_remove_window(const xcb_window_t win)
{
    client_t *c = client_locate(win);
    if (!c)
        return;

    monitor_t *m = c->mon;
    client_remove(c);
    tile(m);
}

void destroy_notify(xcb_generic_event_t *evt)
{
    xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *)evt;
    PRINTF("destroy notify %u\n", e->window);
    event_remove_window(e->window);
}

void unmap_notify(xcb_generic_event_t *evt)
{
    xcb_unmap_notify_event_t *e = (xcb_unmap_notify_event_t *)evt;
    PRINTF("unmap notify: %u\n", e->window);
    event_remove_window(e->window);
}

/**
 * draws and clears a rectangle
 * representing the position of
 * the moved or resized window.
 */
static
void stage_window(unsigned int button, int xw, int yh)
{
    static xcb_rectangle_t r;
    static unsigned int butt;

    const xcb_gcontext_t gc = xcb_generate_id(cfg.conn);
    const uint32_t mask = XCB_GC_FUNCTION | XCB_GC_LINE_WIDTH | XCB_GC_SUBWINDOW_MODE;
    const uint32_t values[] = { XCB_GX_INVERT, cfg.flist->mon->border, XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS };
    xcb_create_gc(cfg.conn, gc, cfg.screen->root, mask, values);

    if (button != XCB_NONE)
        butt = button;
    else /* clear previous rectangle */
        xcb_poly_rectangle(cfg.conn, cfg.screen->root, gc, 1, &r);

    const uint16_t half_border = cfg.flist->mon->border / 2;
    r = (const xcb_rectangle_t) {
        .x = cfg.flist->geom.x + half_border,
        .y = cfg.flist->geom.y + half_border,
        .width  = cfg.flist->geom.width  + cfg.flist->mon->border,
        .height = cfg.flist->geom.height + cfg.flist->mon->border,
    };

    switch (butt) {
        case BUTTON_MOVE:
            r.x = xw - cfg.flist->geom.x + half_border;
            r.y = yh - cfg.flist->geom.y + half_border;
            break;
        case BUTTON_RESIZE:
            r.width  += xw;
            r.height += yh;
            break;
    }

    /* draw new rectangle */
    xcb_poly_rectangle(cfg.conn, cfg.screen->root, gc, 1, &r);
}

void button_press(xcb_generic_event_t *evt)
{
    xcb_button_press_event_t *e = (xcb_button_press_event_t *)evt;

    PRINTF("button '%u' pressed on child '%u' at root (%d,%d)\n", e->detail, e->child, e->root_x, e->root_y);

    client_t *c = (void *)0;
    if (!e->child || !(c = client_locate(e->child))) {
        pointer_ungrab();
        return;
    }

    client_focus(c);

    switch (e->detail) {
        case BUTTON_MOVE:
            c->geom.x = e->root_x - c->geom.x;
            c->geom.y = e->root_y - c->geom.y;
            break;
        case BUTTON_RESIZE:
            c->geom.width  = c->geom.width  - e->root_x;
            c->geom.height = c->geom.height - e->root_y;
            break;
    }
    stage_window(e->detail, e->root_x, e->root_y);

    if (!pointer_grab(pointer_get_by_id(POINTER_TCORSS)))
        warn("failed to grab pointer over window: %u\n", c->win);
}

void button_release(xcb_generic_event_t *evt)
{
    xcb_button_release_event_t *e = (xcb_button_release_event_t *)evt;

    PRINTF("button '%u' released on child '%u' at root (%d,%d)\n", e->detail, e->child, e->root_x, e->root_y);

    stage_window(e->detail, e->root_x, e->root_y);
    switch (e->detail) {
        case BUTTON_MOVE:
            client_move(cfg.flist, e->root_x - cfg.flist->geom.x, e->root_y - cfg.flist->geom.y);
            break;
        case BUTTON_RESIZE:
            client_resize(cfg.flist, e->root_x + cfg.flist->geom.width, e->root_y + cfg.flist->geom.height);
            break;
    }

    pointer_ungrab();
}

void motion_notify(xcb_generic_event_t *evt)
{
    xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *)evt;

    PRINTF("pointer on child '%u' at root (%d,%d)\n", e->child, e->root_x, e->root_y);

    if (!cfg.flist->is_floating) {
        cfg.flist->is_floating = true;
        client_raise(cfg.flist);
        tile(cfg.flist->mon);
    }

    stage_window(XCB_NONE, e->root_x, e->root_y);
}

/**
 * set the event mask for the root window
 *
 * the event mask defines for which events
 * we will be notified about.
 * if setting the event mask fails, then
 * another window manager is running, as
 * XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
 * can be only be set by one client.
 */
void register_root_events(void)
{
    const uint32_t values[] = { ROOT_EVENT_MASK };
    const xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values);
    const xcb_generic_error_t *error = xcb_request_check(cfg.conn, cookie);

    if (error) {
        xcb_disconnect(cfg.conn);
        err("another window manager is already running\n");
    }
}

/**
 * listen for mouse events
 */
void register_mouse_events(void)
{
    xcb_grab_button(cfg.conn, false, cfg.screen->root, BUTTON_MASK, XCB_GRAB_MODE_SYNC,
                    XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, BUTTON_MOVE, BUTTON_MOD);
    xcb_grab_button(cfg.conn, false, cfg.screen->root, BUTTON_MASK, XCB_GRAB_MODE_SYNC,
                    XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, BUTTON_RESIZE, BUTTON_MOD);
}

void handle_event(xcb_generic_event_t *evt)
{
    switch (XCB_EVENT_RESPONSE_TYPE(evt)) {
        case XCB_CONFIGURE_REQUEST: configure_request(evt); break;
        case XCB_MAP_REQUEST:       map_request(evt);       break;
        case XCB_CLIENT_MESSAGE:    client_message(evt);    break;
        case XCB_PROPERTY_NOTIFY:   property_notify(evt);   break;
        case XCB_UNMAP_NOTIFY:      unmap_notify(evt);      break;
        case XCB_DESTROY_NOTIFY:    destroy_notify(evt);    break;
        case XCB_BUTTON_PRESS:      button_press(evt);      break;
        case XCB_BUTTON_RELEASE:    button_release(evt);    break;
        case XCB_MOTION_NOTIFY:     motion_notify(evt);     break;
        default:                    /* not handled */       break;
    }
}

