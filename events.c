#include <stdint.h>

#include "events.h"
#include "global.h"
#include "helpers.h"
#include "window.h"
#include "ewmh.h"
#include "tile.h"

void configure_request(xcb_generic_event_t *evt)
{
    xcb_configure_request_event_t *e = (xcb_configure_request_event_t *)evt;

    PRINTF("configure request %u\n", e->window);

    client_t *c = client_locate(e->window);
    if (c && IS_TILED(c)) {
        xcb_rectangle_t geom = IS_TILED(c) ? c->geom : c->mon->geom;
        xcb_configure_notify_event_t evt = {
            .response_type  = XCB_CONFIGURE_NOTIFY,
            .event          = e->window,
            .window         = e->window,
            .above_sibling  = XCB_NONE,
            .x      = geom.x,
            .y      = geom.y,
            .width  = geom.width,
            .height = geom.height,
            .border_width = 3,
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
    }
}

void map_request(xcb_generic_event_t *evt)
{
    const xcb_map_request_event_t *e = (xcb_map_request_event_t *)evt;

    PRINTF("map request %u\n", e->window);

    if (client_locate(e->window) || window_override_redirect(e->window) || ewmh_wm_type_ignored(e->window))
        return;

    client_t *c = client_create(e->window);
    if (!c)
        err("failed to allocate client for window: %u\n", e->window);

    PRINTF("client name: %s\n", c->name);
    PRINTF("client win : %u\n", c->win);
    PRINTF("client tags: %x\n", c->tags);
    PRINTF("client tran: %s\n", BOOLSTR(c->is_floating));
    PRINTF("client full: %s\n", BOOLSTR(c->is_fullscrn));
    PRINTF("client x   : %u\n", c->geom.x);
    PRINTF("client y   : %u\n", c->geom.y);
    PRINTF("client w   : %u\n", c->geom.width);
    PRINTF("client h   : %u\n", c->geom.height);

    const uint32_t values[] = { XCB_EVENT_MASK_PROPERTY_CHANGE };
    xcb_change_window_attributes(cfg.conn, c->win, XCB_CW_EVENT_MASK, values);

    /* FIXME apply_rules(c); */
    client_add(c);

    if (IS_VISIBLE(c)) {
        tile(c->mon);
        window_show(c->win);
        cfg.client_cur = c;
    }
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
        if (IS_VISIBLE(c))
            cfg.client_cur = c;
    }
}

void property_notify(xcb_generic_event_t *evt)
{
    xcb_property_notify_event_t *e = (xcb_property_notify_event_t *)evt;

    PRINTF("property notify: %u\n", e->window);

    client_t *c = client_locate(e->window);
    if (e->atom != XCB_ATOM_WM_HINTS || !c || c == cfg.client_cur)
        return;

    c->is_urgent = window_is_urgent(c->win);
}

void remove_client(xcb_window_t win)
{
    PRINTF("removing client: %u\n", win);

    client_t *c = client_locate(win);
    if (c) {
        client_unlink(c);
        tile(c->mon);
        free(c);
    }
}

void unmap_notify(xcb_generic_event_t *evt)
{
    xcb_unmap_notify_event_t *e = (xcb_unmap_notify_event_t *)evt;

    PRINTF("unmap notify: %u\n", e->window);

    remove_client(e->window);
}

void destroy_notify(xcb_generic_event_t *evt)
{
    xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *)evt;

    PRINTF("destroy notify %u\n", e->window);

    remove_client(e->window);
}

void rec(unsigned int button, int x, int y)
{
    static xcb_rectangle_t curr;
    static int butt;

    xcb_gcontext_t gc = xcb_generate_id(cfg.conn);
    xcb_create_gc(cfg.conn, gc, cfg.screen->root, XCB_GC_FUNCTION | XCB_GC_LINE_WIDTH, (uint32_t []){ XCB_GX_INVERT, 8 });

    if (button != XCB_NONE) {
        butt = button;
        curr = cfg.client_cur->geom;
    }

    /* clear previous rectangle */
    xcb_poly_rectangle(cfg.conn, cfg.screen->root, gc, 1, &curr);

    curr = cfg.client_cur->geom;

    switch (butt) {
        case BUTTON_MOVE:
            curr.x = x - cfg.client_cur->geom.x;
            curr.y = y - cfg.client_cur->geom.y;
            break;
        case BUTTON_RESIZE:
            curr.width  = x + cfg.client_cur->geom.width;
            curr.height = y + cfg.client_cur->geom.height;
            break;
    }

    /* draw new rectangle */
    xcb_poly_rectangle(cfg.conn, cfg.screen->root, gc, 1, &curr);
}

void motion_notify(xcb_generic_event_t *evt)
{
    xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *)evt;

    PRINTF("pointer moved to root '%u' (%d,%d) event '%u' (%d,%d) upon child '%u' with state: %u\n",
            e->root, e->root_x, e->root_y, e->event, e->event_x, e->event_y, e->child, e->state);

    client_t *c = cfg.client_cur;
    if (!c)
        return;

    if (!c->is_floating) {
        c->is_floating = true;
        tile(c->mon);
    }

    rec(XCB_NONE, e->root_x, e->root_y);

    // xcb_rectangle_t rectangle = cfg.client_cur->geom;
    // if (BITMASK_CHECK(e->state, XCB_EVENT_MASK_BUTTON_1_MOTION)) {
    //     rectangle.x = e->root_x - c->geom.x;
    //     rectangle.y = e->root_y - c->geom.y;
    // } else if (BITMASK_CHECK(e->state, XCB_EVENT_MASK_BUTTON_3_MOTION)) {
    //     rectangle.width += e->root_x;
    //     rectangle.height += e->root_y;
    // }

    // /* FIXME clear rectangle */
    // uint32_t values[] = { XCB_GX_INVERT, XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS};
    // uint32_t mask = XCB_GC_FUNCTION | XCB_GC_SUBWINDOW_MODE;
    // xcb_gcontext_t gc = xcb_generate_id(cfg.conn);
    // xcb_create_gc(cfg.conn, gc, cfg.screen->root, mask, values);
    // xcb_poly_rectangle(cfg.conn, cfg.screen->root, gc, 1, &rectangle);
}

void button_press(xcb_generic_event_t *evt)
{
    xcb_button_press_event_t *e = (xcb_button_press_event_t *)evt;

    PRINTF("button '%u' pressed on event '%u' child '%u' at root (%d,%d) event (%d,%d) with state: %u\n",
            e->detail, e->event, e->child, e->root_x, e->root_y, e->event_x, e->event_y, e->state);

    client_t *c = (void *)0;
    if (!e->child || !(c = client_locate(e->child))) {
        window_ungrab_pointer();
        return;
    }

    cfg.client_cur = c;

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
    rec(e->detail, e->root_x, e->root_y);

    if (!window_grab_pointer(window_get_pointer(POINTER_TCORSS)))
        warn("failed to grab pointer over window: %u\n", c->win);
}

void button_release(xcb_generic_event_t *evt)
{
    xcb_button_release_event_t *e = (xcb_button_release_event_t *)evt;

    PRINTF("button '%u' released on event '%u' child '%u' at root (%d,%d) event (%d,%d) with state: %u\n",
            e->detail, e->event, e->child, e->root_x, e->root_y, e->event_x, e->event_y, e->state);

    client_t *c = cfg.client_cur;
    if (!c)
        return;

    rec(e->detail, e->root_x, e->root_y);
    switch (e->detail) {
        case BUTTON_MOVE:
            client_move(c, e->root_x - c->geom.x, e->root_y - c->geom.y);
            break;
        case BUTTON_RESIZE:
            client_resize(c, e->root_x + c->geom.width, e->root_y + c->geom.height);
            break;
    }

    window_ungrab_pointer();
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
    xcb_flush(cfg.conn);
}

