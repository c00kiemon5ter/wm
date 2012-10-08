#include <xcb/randr.h>
#include <xcb/xinerama.h>

#include "screen.h"
#include "global.h"
#include "helpers.h"

bool randr(void)
{
    /* TODO */
    return false;
}

bool xinerama(void)
{
    bool is_active = false;

    if (xcb_get_extension_data(cfg.connection, &xcb_xinerama_id)->present) {
        xcb_xinerama_is_active_reply_t *reply;
        reply = xcb_xinerama_is_active_reply(cfg.connection, xcb_xinerama_is_active(cfg.connection), NULL);
        is_active = reply->state;
        free(reply);
    }

    if (!is_active) {
        warn("xinerama is not active\n");
        return false;
    }

    xcb_xinerama_query_screens_cookie_t cookie = xcb_xinerama_query_screens_unchecked(cfg.connection);
    xcb_xinerama_query_screens_reply_t *reply = xcb_xinerama_query_screens_reply(cfg.connection, cookie, NULL);
    xcb_xinerama_screen_info_t *info = xcb_xinerama_query_screens_screen_info(reply);
    int nscreens = xcb_xinerama_query_screens_screen_info_length(reply);

    /* FIXME */
    for (int screen = 0; screen < nscreens; screen++) {
        if (screen == cfg.default_screen)
            printf("------ DEFAULT ------\n");
        printf("------ screen info %d -------\n", screen);
        printf("x: %5d\ty: %5d\n", info[screen].x_org, info[screen].y_org);
        printf("w: %5d\th: %5d\n", info[screen].width, info[screen].height);
    }

    return true;
}

void zaphod(void)
{
    /* TODO */
}

