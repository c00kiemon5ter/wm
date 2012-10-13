#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xcb_event.h>

void handle_event(xcb_generic_event_t *);

#endif
