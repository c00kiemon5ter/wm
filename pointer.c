#include <xcb/xcb.h>

#include "pointer.h"
#include "global.h"

#define POINTER_GRAB_MASK   ( XCB_EVENT_MASK_BUTTON_PRESS   \
                            | XCB_EVENT_MASK_BUTTON_RELEASE \
                            | XCB_EVENT_MASK_POINTER_MOTION )

bool window_grab_pointer(xcb_cursor_t cursor)
{
    const xcb_grab_pointer_cookie_t cookie = xcb_grab_pointer_unchecked(cfg.conn, false, cfg.screen->root, POINTER_GRAB_MASK,
                                                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, cursor, XCB_CURRENT_TIME);
    xcb_grab_pointer_reply_t *reply = xcb_grab_pointer_reply(cfg.conn, cookie, (void *)0);

    xcb_free_cursor(cfg.conn, cursor);

    if (!reply)
        return false;

    free(reply);

    return true;
}

inline
void window_ungrab_pointer(void)
{
    xcb_ungrab_pointer(cfg.conn, XCB_CURRENT_TIME);
}

xcb_cursor_t window_get_pointer(const uint16_t pointer_id)
{
    xcb_font_t font = xcb_generate_id(cfg.conn);
    xcb_open_font(cfg.conn, font, sizeof("cursor"), "cursor");

    xcb_cursor_t cursor = xcb_generate_id(cfg.conn);
    xcb_create_glyph_cursor(cfg.conn, cursor, font, font, pointer_id, pointer_id + 1, 0xFFFF,0xFFFF,0xFFFF, 0,0,0);

    xcb_close_font(cfg.conn, font);

    return cursor;
}

