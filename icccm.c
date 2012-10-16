#include <string.h>
#include <xcb/xcb_icccm.h>

#include "icccm.h"
#include "global.h"
#include "helpers.h"

bool icccm_get_window_name(const xcb_window_t win, char *name)
{
    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_name_unchecked(cfg.conn, win);
    xcb_icccm_get_text_property_reply_t data;

    if (!xcb_icccm_get_wm_name_reply(cfg.conn, cookie, &data, (void *)0))
        return false;

    size_t len = min(sizeof(name), data.name_len + 1);
    memcpy(name, data.name, len);
    name[len] = 0;

    PRINTF("Title strlen: %u\n", data.name_len);
    PRINTF("Title string: %s\n", data.name);
    PRINTF("Title name  : %s\n", name);

    xcb_icccm_get_text_property_reply_wipe(&data);

    return true;
}

bool icccm_is_transient(const xcb_window_t win)
{
    xcb_window_t transient = XCB_NONE;

    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_transient_for_unchecked(cfg.conn, win);
    xcb_icccm_get_wm_transient_for_reply(cfg.conn, cookie, &transient, (void *)0);

    return transient != XCB_NONE;
}

