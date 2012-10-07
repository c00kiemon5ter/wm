#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

#include "events.h"

void handle_event(xcb_generic_event_t *evt)
{
    switch (XCB_EVENT_RESPONSE_TYPE(evt)) {
        case XCB_MAP_REQUEST:
            puts("map_request(evt)");
            break;
        case XCB_DESTROY_NOTIFY:
            puts("destroy_notify(evt)");
            break;
        case XCB_UNMAP_NOTIFY:
            puts("unmap_notify(evt)");
            break;
        case XCB_CLIENT_MESSAGE:
            puts("client_message(evt)");
            break;
        case XCB_CONFIGURE_REQUEST:
            puts("configure_request(evt)");
            break;
        case XCB_PROPERTY_NOTIFY:
            puts("property_notify(evt)");
            break;
        case XCB_BUTTON_PRESS:
            puts("button_press(evt)");
            break;
        case XCB_MOTION_NOTIFY:
            puts("motion_notify(evt)");
            break;
        case XCB_BUTTON_RELEASE:
            puts("button_release()");
            break;
        default:
            break;
    }
}

