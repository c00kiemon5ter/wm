#include <stdint.h>

#include "events.h"
#include "global.h"
#include "helpers.h"
#include "window.h"
#include "ewmh.h"
#include "tile.h"

void configure_request(xcb_generic_event_t *evt)
{
    xcb_configure_request_event_t *e = (xcb_configure_request_event_t *) evt;

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
    const xcb_map_request_event_t *e = (xcb_map_request_event_t *) evt;

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
    xcb_client_message_event_t *e = (xcb_client_message_event_t *) evt;

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
            *cfg.cur_client = c;
    }
}

void property_notify(xcb_generic_event_t *evt)
{
    (void)evt;
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
    xcb_unmap_notify_event_t *e = (xcb_unmap_notify_event_t *) evt;

    PRINTF("unmap notify: %u\n", e->window);

    remove_client(e->window);
}

void destroy_notify(xcb_generic_event_t *evt)
{
    xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *) evt;

    PRINTF("destroy notify %u\n", e->window);

    remove_client(e->window);
}

void button_press(xcb_generic_event_t *evt)
{
    (void)evt;
}

void button_release(xcb_generic_event_t *evt)
{
    (void)evt;
}

void motion_notify(xcb_generic_event_t *evt)
{
    (void)evt;
}

/* FIXME implement handlers */
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

