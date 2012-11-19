#include <string.h>

#include "window.h"
#include "wm.h"
#include "icccm.h"

#define CONFIG_WINDOW_MOVE              XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#define CONFIG_WINDOW_RESIZE            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define CONFIG_WINDOW_MOVE_RESIZE       CONFIG_WINDOW_MOVE | CONFIG_WINDOW_RESIZE

/* ** window move and resize function ** */

inline
void window_move(const xcb_window_t win, const int16_t x, const int16_t y)
{
    const uint32_t values[] = { x, y };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_MOVE, values);
}

inline
void window_resize(const xcb_window_t win, const uint16_t width, const uint16_t height)
{
    const uint32_t values[] = { width, height };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_RESIZE, values);
}

inline
void window_move_resize(const xcb_window_t win, const int16_t x, const int16_t y, const uint16_t width, const uint16_t height)
{
    const uint32_t values[] = { x, y, width, height };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_MOVE_RESIZE, values);
}

inline
void window_move_resize_geom(const xcb_window_t win, const xcb_rectangle_t geom)
{
    const uint32_t values[] = { geom.x, geom.y, geom.width, geom.height };
    xcb_configure_window(cfg.conn, win, CONFIG_WINDOW_MOVE_RESIZE, values);
}

bool window_update_geom(const xcb_window_t win, xcb_rectangle_t *geom)
{
    const xcb_get_geometry_cookie_t cookie = xcb_get_geometry(cfg.conn, win);
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    geom->x = reply->x;
    geom->y = reply->y;
    geom->width = reply->width;
    geom->height = reply->height;

    free(reply);

    return true;
}

bool window_override_redirect(const xcb_window_t win)
{
    const xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(cfg.conn, win);
    xcb_get_window_attributes_reply_t *reply = xcb_get_window_attributes_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    const bool state = reply->override_redirect;

    free(reply);

    return state;
}

inline
bool window_is_urgent(const xcb_window_t win)
{
    return icccm_has_urgent_hint(win);
}

inline
void window_set_border_width(xcb_window_t win, const uint16_t border_width)
{
    const uint32_t values[] = { border_width };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
}

/* ** visibility functions ** */

void window_set_visibility(const xcb_window_t win, bool visible)
{
    uint32_t values_off[] = { ROOT_EVENT_MASK & ~XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY };
    uint32_t values_on[]  = { ROOT_EVENT_MASK };

    xcb_grab_server(cfg.conn);

    xcb_change_window_attributes(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values_off);
    if (visible)
        xcb_map_window(cfg.conn, win);
    else
        xcb_unmap_window(cfg.conn, win);
    xcb_change_window_attributes(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values_on);

    xcb_ungrab_server(cfg.conn);
}

inline
void window_show(const xcb_window_t win)
{
    window_set_visibility(win, true);
}

inline
void window_hide(const xcb_window_t win)
{
    window_set_visibility(win, false);
}

/* ** window stacking order functions ** */

void window_raise(const xcb_window_t win)
{
    const uint32_t values[] = { XCB_STACK_MODE_ABOVE };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
}

void window_lower(const xcb_window_t win)
{
    const uint32_t values[] = { XCB_STACK_MODE_BELOW };
    xcb_configure_window(cfg.conn, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
}

