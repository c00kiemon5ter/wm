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
    xcb_randr_get_screen_resources_reply_t *reply = xcb_randr_get_screen_resources_reply(cfg.connection, cookie, NULL);
    xcb_randr_crtc_t *info = xcb_randr_get_screen_resources_crtcs(reply);

    PRINTF("randr crtcs: %d\n", reply->num_crtcs);

    for (int crtc = 0; crtc < reply->num_crtcs; crtc++) {
        xcb_randr_get_crtc_info_cookie_t cookie = xcb_randr_get_crtc_info(cfg.connection, info[crtc], XCB_CURRENT_TIME);
        xcb_randr_get_crtc_info_reply_t *reply = xcb_randr_get_crtc_info_reply(cfg.connection, cookie, NULL);

        int outputs = xcb_randr_get_crtc_info_outputs_length(reply);
        if (!outputs)
            continue;

        PRINTF("info for crtc: %d\n", crtc);
        PRINTF("x: %5d\ty: %5d\n", reply->x,     reply->y);
        PRINTF("w: %5d\th: %5d\n", reply->width, reply->height);

        /* FIXME init monitor struct */
        int x = reply->x;
        int y = reply->y;
        int w = reply->width;
        int h = reply->height;

        /* FIXME do we need this ? */
        xcb_randr_output_t *info = xcb_randr_get_crtc_info_outputs(reply);

        PRINTF("outputs: %d\n", outputs);

        for (int output = 0; output < outputs; output++) {
            xcb_randr_get_output_info_cookie_t cookie = xcb_randr_get_output_info(cfg.connection, info[output], XCB_CURRENT_TIME);
            xcb_randr_get_output_info_reply_t *reply = xcb_randr_get_output_info_reply(cfg.connection, cookie, NULL);

            int name_len = xcb_randr_get_output_info_name_length(reply);
            //char *name = memcpy(&name, xcb_randr_get_output_info_name(reply), name_len);
            //name[name_len] = 0;

            PRINTF("info for output: %d\n", output);
            //PRINTF("name: %s\n", name);
            PRINTF("name length: %zd\n", name_len);
            PRINTF("mm_width: %5d\tmm_height: %5d\n", reply->mm_width, reply->mm_height);

            if (reply)
                free(reply);
        }
        /* up to here */

        if (reply)
            free(reply);
    }

    if (reply)
        free(reply);

    return true;
}

bool xinerama(void)
{
    bool is_active = false;

    if (xcb_get_extension_data(cfg.connection, &xcb_xinerama_id)->present) {
        xcb_xinerama_is_active_reply_t *reply = xcb_xinerama_is_active_reply(cfg.connection, xcb_xinerama_is_active(cfg.connection), NULL);
        is_active = reply->state;
        if (reply)
            free(reply);
    }

    if (!is_active) {
        warn("xinerama is not present\n");
        return false;
    }

    xcb_xinerama_query_screens_cookie_t cookie = xcb_xinerama_query_screens_unchecked(cfg.connection);
    xcb_xinerama_query_screens_reply_t *reply = xcb_xinerama_query_screens_reply(cfg.connection, cookie, NULL);
    xcb_xinerama_screen_info_t *info = xcb_xinerama_query_screens_screen_info(reply);
    int screens = xcb_xinerama_query_screens_screen_info_length(reply);

    PRINTF("xinerama screens: %d\n", screens);

    for (int screen = 0; screen < screens; screen++) {
        /* FIXME init monitor struct */

        PRINTF("info for screen: %d\n", screen);
        PRINTF("x: %5d\ty: %5d\n", info[screen].x_org, info[screen].y_org);
        PRINTF("w: %5d\th: %5d\n", info[screen].width, info[screen].height);
    }

    if (reply)
        free(reply);

    return true;
}

void zaphod(void)
{
    /* FIXME init monitor struct */
    int x = 0;
    int y = 0;
    int w = cfg.screen->width_in_pixels;
    int h = cfg.screen->height_in_pixels;
}

