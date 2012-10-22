#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xcb_event.h>

#define BUTTON_MOVE             XCB_BUTTON_INDEX_1
#define BUTTON_RESIZE           XCB_BUTTON_INDEX_3
#define BUTTON_MOD              XCB_MOD_MASK_4
#define BUTTON_MASK             ( XCB_EVENT_MASK_BUTTON_PRESS   \
                                | XCB_EVENT_MASK_BUTTON_RELEASE )

void register_root_events(void);
void register_mouse_events(void);
void handle_event(xcb_generic_event_t *);

#endif
