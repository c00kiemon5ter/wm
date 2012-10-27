#include <string.h>

#include "ewmh.h"
#include "global.h"
#include "helpers.h"

void ewmh_init(void)
{
    if (!(cfg.ewmh = calloc(1, sizeof(xcb_ewmh_connection_t))))
        err("failed to allocate ewmh object\n");

    xcb_intern_atom_cookie_t *cookie = xcb_ewmh_init_atoms(cfg.conn, cfg.ewmh);
    xcb_ewmh_init_atoms_replies(cfg.ewmh, cookie, (void *)0);
}

void ewmh_set_supported_atoms(void)
{
    xcb_atom_t new_atoms[] = {
        cfg.ewmh->_NET_SUPPORTED,
        cfg.ewmh->_NET_WM_STATE,
        cfg.ewmh->_NET_WM_STATE_FULLSCREEN,
        cfg.ewmh->_NET_WM_WINDOW_TYPE,
        cfg.ewmh->_NET_WM_WINDOW_TYPE_DIALOG,
        cfg.ewmh->_NET_WM_WINDOW_TYPE_UTILITY,
        cfg.ewmh->_NET_WM_WINDOW_TYPE_TOOLBAR,
        cfg.ewmh->_NET_DESKTOP_NAMES,
        cfg.ewmh->_NET_NUMBER_OF_DESKTOPS,
        cfg.ewmh->_NET_CURRENT_DESKTOP,
        cfg.ewmh->_NET_CLIENT_LIST,
        cfg.ewmh->_NET_ACTIVE_WINDOW,
    };
    xcb_ewmh_set_supported(cfg.ewmh, cfg.def_screen, LENGTH(new_atoms), new_atoms);
}

inline
void ewmh_update_active_window(const xcb_window_t win)
{
    xcb_ewmh_set_active_window(cfg.ewmh, cfg.def_screen, (win == 0) ? XCB_NONE : win);
}

inline
void ewmh_update_wm_name(const char *wm_name)
{
    xcb_ewmh_set_wm_name(cfg.ewmh, cfg.screen->root, strlen(wm_name), wm_name);
}

bool ewmh_get_window_title(const xcb_window_t win, char *name)
{
    const xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_name_unchecked(cfg.ewmh, win);
    xcb_ewmh_get_utf8_strings_reply_t data;

    if (!xcb_ewmh_get_wm_name_reply(cfg.ewmh, cookie, &data, (void *)0))
        return false;

    size_t len = min(sizeof(name), data.strings_len + 1);
    memcpy(name, data.strings, len);
    name[len] = 0;

    xcb_ewmh_get_utf8_strings_reply_wipe(&data);

    return true;
}

bool ewmh_wm_state_fullscreen(const xcb_window_t win)
{
    const xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_state_unchecked(cfg.ewmh, win);
    xcb_ewmh_get_atoms_reply_t data;

    if (!xcb_ewmh_get_wm_state_reply(cfg.ewmh, cookie, &data, (void *)0))
        return false;

    bool state = false;
    for (unsigned short i = 0; i < data.atoms_len; i++)
        if ((state = data.atoms[i] == cfg.ewmh->_NET_WM_STATE_FULLSCREEN))
            break;

    xcb_ewmh_get_atoms_reply_wipe(&data);

    return state;
}

bool ewmh_wm_type_dialog(const xcb_window_t win)
{
    const xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_window_type_unchecked(cfg.ewmh, win);
    xcb_ewmh_get_atoms_reply_t data;

    if (!xcb_ewmh_get_wm_window_type_reply(cfg.ewmh, cookie, &data, (void *)0))
        return false;

    bool state = false;
    for (unsigned short i = 0; i < data.atoms_len; i++)
        if ((state = data.atoms[i] == cfg.ewmh->_NET_WM_WINDOW_TYPE_DIALOG))
            break;

    xcb_ewmh_get_atoms_reply_wipe(&data);

    return state;
}

bool ewmh_wm_type_ignored(const xcb_window_t win)
{
    const xcb_get_property_cookie_t cookie = xcb_ewmh_get_wm_window_type_unchecked(cfg.ewmh, win);
    xcb_ewmh_get_atoms_reply_t data;

    if (!xcb_ewmh_get_wm_window_type_reply(cfg.ewmh, cookie, &data, (void *)0))
        return false;

    bool state = false;
    for (unsigned short i = 0; i < data.atoms_len; i++)
        if ((state = data.atoms[i] == cfg.ewmh->_NET_WM_WINDOW_TYPE_UTILITY
                  || data.atoms[i] == cfg.ewmh->_NET_WM_WINDOW_TYPE_DOCK
                  || data.atoms[i] == cfg.ewmh->_NET_WM_WINDOW_TYPE_NOTIFICATION))
            break;

    xcb_ewmh_get_atoms_reply_wipe(&data);

    return state;
}

