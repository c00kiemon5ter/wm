#include <string.h>
#include <stdint.h>

#include <xcb/randr.h>
#include <xcb/xinerama.h>

#include "monitor.h"
#include "helpers.h"

/**
 * add a new monitor to the tail of the monitor list
 */
void monitor_add(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
    monitor_t **m = &cfg.monitors;
    while (*m) m = &(*m)->next;

    if (!(*m = calloc(1, sizeof(monitor_t))))
        err("failed to allocate monitor\n");

    (*m)->geom = (const xcb_rectangle_t){ .x = x, .y = y, .width = w, .height = h };

    PRINTF("x: %5d\n", (*m)->geom.x);
    PRINTF("y: %5d\n", (*m)->geom.y);
    PRINTF("w: %5u\n", (*m)->geom.width);
    PRINTF("h: %5u\n", (*m)->geom.height);
}

/**
 * focus the given monitor
 *
 * the focused/active monitor is always
 * the first monitor on the monitor list
 */
void monitor_focus(monitor_t *mon)
{
    if (!mon || cfg.monitors == mon)
        return;

    monitor_t **m = &cfg.monitors;
    while (*m && *m != mon) m = &(*m)->next;
    *m = mon->next;

    mon->next = cfg.monitors;
    cfg.monitors = mon;
}

void monitor_focus_next(void)
{
    monitor_t *m = cfg.monitors->next;

    if (!m)
        return;

    monitor_focus(m);
}

void monitor_focus_prev(void)
{
    monitor_t *m = cfg.monitors;
    while (m && m->next) m = m->next;
    monitor_focus(m);
}

/* ** xcb related functions - monitor initialization ** */

bool randr(void)
{
    if (!xcb_get_extension_data(cfg.conn, &xcb_randr_id)->present) {
        warn("randr is not present\n");
        return false;
    }

    const xcb_randr_get_screen_resources_cookie_t cookie = xcb_randr_get_screen_resources_unchecked(cfg.conn, cfg.screen->root);
    xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    const xcb_randr_crtc_t *info = xcb_randr_get_screen_resources_crtcs(reply);

    PRINTF("randr num crtcs: %u\n", reply->num_crtcs);

    xcb_randr_get_crtc_info_cookie_t cookies[reply->num_crtcs];
    for (uint16_t crtc = 0; crtc < reply->num_crtcs; crtc++)
        cookies[crtc] = xcb_randr_get_crtc_info_unchecked(cfg.conn, info[crtc], XCB_CURRENT_TIME);

    for (uint16_t crtc = 0; crtc < reply->num_crtcs; crtc++) {
        xcb_randr_get_crtc_info_reply_t *reply = xcb_randr_get_crtc_info_reply(cfg.conn, cookies[crtc], (void *)0);

        if (!reply)
            continue;

        PRINTF("adding crtc: %u\n", crtc);

        monitor_add(reply->x, reply->y, reply->width, reply->height);
        free(reply);
    }

    free(reply);

    return true;
}

bool xinerama(void)
{
    bool is_active = false;

    if (xcb_get_extension_data(cfg.conn, &xcb_xinerama_id)->present) {
        xcb_xinerama_is_active_reply_t *reply = xcb_xinerama_is_active_reply(cfg.conn, xcb_xinerama_is_active(cfg.conn), (void *)0);

        if (!reply)
            return false;

        is_active = reply->state;
        free(reply);
    }

    if (!is_active) {
        warn("xinerama is not present\n");
        return false;
    }

    const xcb_xinerama_query_screens_cookie_t cookie = xcb_xinerama_query_screens_unchecked(cfg.conn);
    xcb_xinerama_query_screens_reply_t *reply = xcb_xinerama_query_screens_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return false;

    const xcb_xinerama_screen_info_t *info = xcb_xinerama_query_screens_screen_info(reply);
    int screens = xcb_xinerama_query_screens_screen_info_length(reply);

    PRINTF("xinerama screens: %d\n", screens);

    for (int screen = 0; screen < screens; screen++) {
        PRINTF("adding screen: %d\n", screen);
        monitor_add(info[screen].x_org, info[screen].y_org, info[screen].width, info[screen].height);
    }

    free(reply);

    return true;
}

inline
void zaphod(void)
{
    monitor_add(0, 0, cfg.screen->width_in_pixels, cfg.screen->height_in_pixels);
}

