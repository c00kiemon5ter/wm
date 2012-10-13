#include <string.h>

#include <xcb/randr.h>
#include <xcb/xinerama.h>

#include "screen.h"
#include "global.h"
#include "helpers.h"

bool randr(void)
{
    if (!xcb_get_extension_data(cfg.connection, &xcb_xinerama_id)->present) {
        warn("randr is not present\n");
        return false;
    }

    xcb_randr_get_screen_resources_cookie_t cookie = xcb_randr_get_screen_resources(cfg.connection, cfg.screen->root);
    xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(cfg.connection, cookie, (void *)0);
    xcb_randr_crtc_t *info = xcb_randr_get_screen_resources_crtcs(reply);

    PRINTF("randr num crtcs: %zd\n", reply->num_crtcs);

    monitor_t **m = &cfg.monitors;

    for (uint16_t crtc = 0; crtc < reply->num_crtcs; crtc++) {
        xcb_randr_get_crtc_info_cookie_t cookie = xcb_randr_get_crtc_info(cfg.connection, info[crtc], XCB_CURRENT_TIME);
        xcb_randr_get_crtc_info_reply_t *reply = xcb_randr_get_crtc_info_reply(cfg.connection, cookie, (void *)0);

        /* if crtc not associated with any monitor, skip it */
        if (xcb_randr_get_crtc_info_outputs_length(reply) == 0)
            continue;

        if (!(*m = calloc(1, sizeof(monitor_t))))
            err("failed to allocate crtc: %d\n", crtc);

        (*m)->geom.x = reply->x;
        (*m)->geom.y = reply->y;
        (*m)->geom.width  = reply->width;
        (*m)->geom.height = reply->height;

        PRINTF("info for crtc: %d\n", crtc);
        PRINTF("x: %5d\n", (*m)->geom.x);
        PRINTF("y: %5d\n", (*m)->geom.y);
        PRINTF("w: %5d\n", (*m)->geom.width);
        PRINTF("h: %5d\n", (*m)->geom.height);

        m = &(*m)->next;

        // xcb_randr_output_t *info = xcb_randr_get_crtc_info_outputs(reply);
        // int outputs = xcb_randr_get_crtc_info_outputs_length(reply);
        //
        // PRINTF("outputs: %d\n", outputs);
        //
        // for (int output = 0; output < outputs; output++) {
        //     xcb_randr_get_output_info_cookie_t cookie = xcb_randr_get_output_info(cfg.connection, info[output], XCB_CURRENT_TIME);
        //     xcb_randr_get_output_info_reply_t *reply = xcb_randr_get_output_info_reply(cfg.connection, cookie, (void *)0);
        //
        //     size_t name_len = xcb_randr_get_output_info_name_length(reply) + 1;
        //     char name[BUFLEN];
        //     snprintf(name, MIN(name_len, sizeof(name)), "%s", xcb_randr_get_output_info_name(reply));
        //
        //     PRINTF("info for output: %d -- %s\n", output, name);
        //     PRINTF("mm_width : %5d\n", reply->mm_width);
        //     PRINTF("mm_height: %5d\n", reply->mm_height);
        //
        //     if (reply)
        //         free(reply);
        // }

        free(reply);
    }

    free(reply);

    return true;
}

bool xinerama(void)
{
    bool is_active = false;

    if (xcb_get_extension_data(cfg.connection, &xcb_xinerama_id)->present) {
        xcb_xinerama_is_active_reply_t *reply = xcb_xinerama_is_active_reply(cfg.connection, xcb_xinerama_is_active(cfg.connection), (void *)0);
        is_active = reply->state;
        free(reply);
    }

    if (!is_active) {
        warn("xinerama is not present\n");
        return false;
    }

    xcb_xinerama_query_screens_cookie_t cookie = xcb_xinerama_query_screens_unchecked(cfg.connection);
    xcb_xinerama_query_screens_reply_t *reply = xcb_xinerama_query_screens_reply(cfg.connection, cookie, (void *)0);
    xcb_xinerama_screen_info_t *info = xcb_xinerama_query_screens_screen_info(reply);
    int screens = xcb_xinerama_query_screens_screen_info_length(reply);

    PRINTF("xinerama screens: %d\n", screens);

    monitor_t **m = &cfg.monitors;

    for (int screen = 0; screen < screens; screen++) {
        if (!(*m = calloc(1, sizeof(monitor_t))))
            err("failed to allocate crtc: %d\n", screen);

        (*m)->geom.x = info[screen].x_org;
        (*m)->geom.y = info[screen].y_org;
        (*m)->geom.width  = info[screen].width;
        (*m)->geom.height = info[screen].height;

        PRINTF("info for screen: %d\n", screen);
        PRINTF("x: %5d\n", (*m)->geom.x);
        PRINTF("y: %5d\n", (*m)->geom.y);
        PRINTF("w: %5d\n", (*m)->geom.width);
        PRINTF("h: %5d\n", (*m)->geom.height);

        m = &(*m)->next;
    }

    free(reply);

    return true;
}

void zaphod(void)
{
    if (!(cfg.monitors = calloc(1, sizeof(monitor_t))))
        err("failed to allocate monitor\n");

    cfg.monitors->geom.x = 0;
    cfg.monitors->geom.y = 0;
    cfg.monitors->geom.width  = cfg.screen->width_in_pixels;
    cfg.monitors->geom.height = cfg.screen->height_in_pixels;

    PRINTF("info for monitor: %d\n", 1);
    PRINTF("x: %5d\n", cfg.monitors->geom.x);
    PRINTF("y: %5d\n", cfg.monitors->geom.y);
    PRINTF("w: %5d\n", cfg.monitors->geom.width);
    PRINTF("h: %5d\n", cfg.monitors->geom.height);

}

