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
        cfg.cur_client = c;
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
            cfg.cur_client = c;
    }
}

void property_notify(xcb_generic_event_t *evt)
{
    xcb_property_notify_event_t *e = (xcb_property_notify_event_t *)evt;

    PRINTF("property notify: %u\n", e->window);

    client_t *c = client_locate(e->window);
    if (e->atom != XCB_ATOM_WM_HINTS || !c || c == cfg.cur_client)
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

void button_press(xcb_generic_event_t *evt)
{
    xcb_button_press_event_t *e = (xcb_button_press_event_t *)evt;

    PRINTF("button '%u' pressed on event '%u' child '%u' at root (%d,%d) event (%d,%d) with state: %u\n",
            e->detail, e->event, e->child, e->root_x, e->root_y, e->event_x, e->event_y, e->state);

    client_t *c = client_locate(e->child);
    if (!c)
        return;

    if (BUTTON_FOCUS == e->detail)
        cfg.cur_client = c;

    if (!window_grab_pointer())
        warn("failed to grab pointer over window: %u\n", e->child);
}

void motion_notify(xcb_generic_event_t *evt)
{
    xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *)evt;

    PRINTF("pointer moved to root '%u' (%d,%d) event '%u' (%d,%d) upon child '%u' with state: %u\n",
            e->root, e->root_x, e->root_y, e->event, e->event_x, e->event_y, e->child, e->state);

    client_t *c = client_locate(e->child);
    if (!c)
        return;

    if (!c->is_floating) {
        c->is_floating = true;
        tile(c->mon);
    }

    if (BITMASK_CHECK(e->state, BUTTON_RESIZE_MOTION)) {
        int16_t mid_x = c->geom.x + (c->geom.width  / 2);
        int16_t mid_y = c->geom.y + (c->geom.height / 2);
        unsigned short cursor_type = { 0 };
        uint16_t font_id = 0;

        /* bitcodes designating cursor position
         * first (lsb) bit is the position on the y axis
         * second bit is the position on the x axis
         *
         *   cursor_type  position
         *   case 0 : 00  TL Top Left
         *   case 1 : 01  TR Top Right
         *   case 2 : 10  BL Bottom Left
         *   case 3 : 11  BR Bottom Right
         */
        if (mid_x < e->root_x)
            BIT_SET(cursor_type, 0);
        if (mid_y < e->root_y)
            BIT_SET(cursor_type, 1);

        switch (cursor_type) {
            case 0: font_id = 134; break;
            case 1: font_id = 136; break;
            case 2: font_id =  12; break;
            case 3: font_id =  14; break;
        }

        xcb_font_t font = xcb_generate_id(cfg.conn);
        xcb_open_font(cfg.conn, font, sizeof("cursor"), "cursor");

        xcb_cursor_t cursor = xcb_generate_id(cfg.conn);
        xcb_create_glyph_cursor(cfg.conn, cursor, font, font, font_id, font_id + 1, 0,0,0, 0xFFFF,0xFFFF,0xFFFF);

        xcb_gcontext_t gc = xcb_generate_id(cfg.conn);
        xcb_create_gc(cfg.conn, gc, c->win, XCB_GC_FONT, &font);

        xcb_change_window_attributes (cfg.conn, c->win, XCB_CW_CURSOR, &cursor);

        xcb_free_cursor(cfg.conn, cursor);
        xcb_close_font(cfg.conn, font);
    }
}

void button_release(xcb_generic_event_t *evt)
{
    xcb_button_release_event_t *e = (xcb_button_release_event_t *)evt;

    PRINTF("button '%u' released on event '%u' child '%u' at root (%d,%d) event (%d,%d) with state: %u\n",
            e->detail, e->event, e->child, e->root_x, e->root_y, e->event_x, e->event_y, e->state);

    client_t *c = client_locate(e->child);
    if (!c)
        return;

    xcb_font_t font = xcb_generate_id(cfg.conn);
    xcb_open_font(cfg.conn, font, sizeof("cursor"), "cursor");

    xcb_cursor_t cursor = xcb_generate_id(cfg.conn);
    xcb_create_glyph_cursor(cfg.conn, cursor, font, font, 68, 68 + 1, 0,0,0, 0xFFFF,0xFFFF,0xFFFF);

    xcb_gcontext_t gc = xcb_generate_id(cfg.conn);
    xcb_create_gc(cfg.conn, gc, c->win, XCB_GC_FONT, &font);

    xcb_change_window_attributes (cfg.conn, c->win, XCB_CW_CURSOR, &cursor);

    xcb_free_cursor(cfg.conn, cursor);
    xcb_close_font(cfg.conn, font);

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
    xcb_grab_button(cfg.conn, false, cfg.screen->root, BUTTON_MASK, XCB_GRAB_MODE_SYNC,
                    XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, BUTTON_FOCUS, BUTTON_MOD);
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

