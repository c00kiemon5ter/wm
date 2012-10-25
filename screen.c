#include <string.h>
#include <stdint.h>

#include <xcb/randr.h>
#include <xcb/xinerama.h>

#include "screen.h"
#include "global.h"
#include "helpers.h"

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

    for (uint16_t crtc = 0; crtc < reply->num_crtcs; crtc++) {
        const xcb_randr_get_crtc_info_cookie_t cookie = xcb_randr_get_crtc_info_unchecked(cfg.conn, info[crtc], XCB_CURRENT_TIME);
        xcb_randr_get_crtc_info_reply_t *reply = xcb_randr_get_crtc_info_reply(cfg.conn, cookie, (void *)0);

        if (!reply)
            continue;

        // xcb_randr_output_t *info = xcb_randr_get_crtc_info_outputs(reply);
        // int outputs = xcb_randr_get_crtc_info_outputs_length(reply);
        //
        // /* if crtc not associated with any monitor, skip it */
        // if (outputs == 0) {
        //     free(reply);
        //     continue;
        // }
        //
        // PRINTF("outputs: %d\n", outputs);
        //
        // for (int output = 0; output < outputs; output++) {
        //     const xcb_randr_get_output_info_cookie_t cookie = xcb_randr_get_output_info_unchecked(cfg.conn, info[output], XCB_CURRENT_TIME);
        //     xcb_randr_get_output_info_reply_t *reply = xcb_randr_get_output_info_reply(cfg.conn, cookie, (void *)0);
        //
        //     if (!reply)
        //         continue;
        //
        //     size_t name_len = xcb_randr_get_output_info_name_length(reply) + 1;
        //     char name[BUFLEN];
        //     snprintf(name, MIN(name_len, sizeof(name)), "%s", xcb_randr_get_output_info_name(reply));
        //
        //     PRINTF("info for output: %d -- %s\n", output, name);
        //     PRINTF("mm_width : %5u\n", reply->mm_width);
        //     PRINTF("mm_height: %5u\n", reply->mm_height);
        //
        //     free(reply);
        // }

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

void zaphod(void)
{
    monitor_add(0, 0, cfg.screen->width_in_pixels, cfg.screen->height_in_pixels);
}

