#include "events.h"
#include "global.h"
#include "helpers.h"
#include "window.h"

void configure_request(xcb_generic_event_t *evt)
{
    xcb_configure_request_event_t *e = (xcb_configure_request_event_t *) evt;

    PRINTF("configure request %u\n", e->window);

    uint16_t mask = 0;
    uint32_t values[7];
    unsigned short i = 0;

    if (e->value_mask & XCB_CONFIG_WINDOW_X) {
        mask |= XCB_CONFIG_WINDOW_X;
        values[i++] = e->x;
    }

    if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
        mask |= XCB_CONFIG_WINDOW_Y;
        values[i++] = e->y;
    }

    if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
        mask |= XCB_CONFIG_WINDOW_WIDTH;
        values[i++] = e->width;
    }

    if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
        mask |= XCB_CONFIG_WINDOW_HEIGHT;
        values[i++] = e->height;
    }

    if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
        mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
        values[i++] = e->border_width;
    }

    if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
        mask |= XCB_CONFIG_WINDOW_SIBLING;
        values[i++] = e->sibling;
    }

    if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
        mask |= XCB_CONFIG_WINDOW_STACK_MODE;
        values[i++] = e->stack_mode;
    }

    xcb_configure_window(cfg.conn, e->window, mask, values);
}

void map_request(xcb_generic_event_t *evt)
{
    const xcb_map_request_event_t *e = (xcb_map_request_event_t *) evt;

    PRINTF("map request %u\n", e->window);

    if (locate(e->window))
        return;

    client_t *c = NULL;
    if (!(c = create_client(e->window)))
        err("failed to allocate client for window: %u\n", e->window);

    PRINTF("client name: %s\n", c->name);

    add_client(c);

    xcb_get_window_attributes_reply_t *wa = NULL;
    wa = xcb_get_window_attributes_reply(cfg.conn, xcb_get_window_attributes(cfg.conn, e->window), NULL);
    if (wa && wa->override_redirect) {
        free(wa);
        return;
    }

    xcb_map_window(cfg.conn, e->window);

    uint32_t values[] = {10, 10, 500, 500};
    xcb_configure_window(cfg.conn, e->window,
            XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y|
            XCB_CONFIG_WINDOW_WIDTH|XCB_CONFIG_WINDOW_HEIGHT, values);
}

void client_message(xcb_generic_event_t *evt)
{
    (void)evt;
}

void property_notify(xcb_generic_event_t *evt)
{
    (void)evt;
}

void unmap_notify(xcb_generic_event_t *evt)
{
    (void)evt;
}

void destroy_notify(xcb_generic_event_t *evt)
{
    (void)evt;
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

