#include <string.h>

#include <xcb/xcb_icccm.h>

#include "wm.h"
#include "icccm.h"
#include "helpers.h"

xcb_atom_t get_atom(const char *atom_name)
{
    xcb_atom_t atom = 0;

    const xcb_intern_atom_cookie_t cookie = xcb_intern_atom_unchecked(cfg.conn, 0, strlen(atom_name), atom_name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(cfg.conn, cookie, (void *)0);

    if (!reply)
        return 0;

    atom = reply->atom;
    free(reply);

    return atom;
}

bool icccm_close_window(const xcb_window_t win)
{
    xcb_atom_t wm_protocols = get_atom(WM_PROTOCOLS);
    xcb_atom_t wm_delete_window = get_atom(WM_DELETE_WINDOW);

    if (!wm_protocols || !wm_delete_window)
        return false;

    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_protocols_unchecked(cfg.conn, win, wm_protocols);
    xcb_icccm_get_wm_protocols_reply_t data;

    if (!xcb_icccm_get_wm_protocols_reply(cfg.conn, cookie, &data, (void *)0))
        return false;

    bool state = false;
    for (unsigned short i = 0; i < data.atoms_len; i++)
        if ((state = data.atoms[i] == wm_delete_window))
            break;

    xcb_icccm_get_wm_protocols_reply_wipe(&data);

    if (state) {
        const xcb_client_message_event_t evt = {
            .response_type = XCB_CLIENT_MESSAGE,
            .window = win,
            .format = 32,
            .type = wm_protocols,
            .data.data32[0] = wm_delete_window,
            .data.data32[1] = XCB_CURRENT_TIME,
        };
        xcb_send_event(cfg.conn, false, win, XCB_EVENT_MASK_NO_EVENT, (char *)&evt);
    }

    PRINTF("close window: %s\n", BOOLSTR(state));

    return state;
}

bool icccm_get_window_class(const xcb_window_t win, char *class, char *instance)
{
    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_class_unchecked(cfg.conn, win);
    xcb_icccm_get_wm_class_reply_t data;

    if (!xcb_icccm_get_wm_class_reply(cfg.conn, cookie, &data, (void *)0))
        return false;

    snprintf(class, sizeof(class), "%s", data.class_name);
    snprintf(instance, sizeof(instance), "%s", data.instance_name);

    xcb_icccm_get_wm_class_reply_wipe(&data);

    return true;
}

bool icccm_get_window_title(const xcb_window_t win, char *name)
{
    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_name_unchecked(cfg.conn, win);
    xcb_icccm_get_text_property_reply_t data;

    if (!xcb_icccm_get_wm_name_reply(cfg.conn, cookie, &data, (void *)0))
        return false;

    size_t len = min(sizeof(name), data.name_len + 1);
    memcpy(name, data.name, len);
    name[len] = 0;

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

bool icccm_has_urgent_hint(const xcb_window_t win)
{
    const xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_hints_unchecked(cfg.conn, win);
    xcb_icccm_wm_hints_t data;

    if (!xcb_icccm_get_wm_hints_reply(cfg.conn, cookie, &data, (void *)0))
        return false;

    return xcb_icccm_wm_hints_get_urgency(&data);
}

