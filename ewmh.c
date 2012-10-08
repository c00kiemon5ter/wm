#include <stdio.h>
#include <string.h>

#include "ewmh.h"
#include "global.h"
#include "helpers.h"

void ewmh_init(void)
{
    cfg.ewmh = malloc(sizeof(xcb_ewmh_connection_t));
    if (cfg.ewmh == NULL)
        err("failed to allocate ewmh object\n");

    xcb_intern_atom_cookie_t *ewmh_cookies = xcb_ewmh_init_atoms(cfg.connection, cfg.ewmh);
    xcb_ewmh_init_atoms_replies(cfg.ewmh, ewmh_cookies, NULL);
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
    xcb_ewmh_set_supported(cfg.ewmh, cfg.default_screen, LENGTH(new_atoms), new_atoms);
}

void ewmh_update_wm_name(const char *wm_name)
{
    snprintf(cfg.wm_name, sizeof(cfg.wm_name), "%s", wm_name);
    xcb_ewmh_set_wm_name(cfg.ewmh, cfg.screen->root, strlen(cfg.wm_name), cfg.wm_name);
}

void ewmh_update_active_window(const xcb_window_t win)
{
    xcb_ewmh_set_active_window(cfg.ewmh, cfg.default_screen, (win == 0) ? XCB_NONE : win);
}

/* FIXME */
void ewmh_update_number_of_desktops(void)
{
//xcb_ewmh_set_number_of_desktops(ewmh, default_screen, num_desktops);
}

/* FIXME */
void ewmh_update_current_desktop(void)
{
   // desktop_t *d = desk_head;
   // unsigned int i = 0, cd = 0;

   // while (d != NULL && i < num_desktops) {
   //     if (desk == d)
   //         cd = i;
   //     i++;
   //     d = d->next;
   // }

   // xcb_ewmh_set_current_desktop(ewmh, default_screen, cd);
}

/* FIXME */
void ewmh_update_desktop_names(void)
{
   // char names[MAXLEN];
   // desktop_t *d = desk_head;
   // unsigned int pos, i;

   // pos = i = 0;

   // while (d != NULL && i < num_desktops) {
   //     for (unsigned int j = 0; j < strlen(d->name); j++)
   //         names[pos + j] = d->name[j];
   //     pos += strlen(d->name);
   //     names[pos] = '\0';
   //     pos++;
   //     d = d->next;
   //     i++;
   // }

   // if (i != num_desktops)
   //     return;

   // pos--;

   // xcb_ewmh_set_desktop_names(ewmh, default_screen, pos, names);
}

/* FIXME */
void ewmh_update_client_list(void)
{
    // if (num_clients == 0) {
    //     xcb_ewmh_set_client_list(ewmh, default_screen, 0, NULL);
    //     return;
    // }

    // xcb_window_t wins[num_clients];
    // desktop_t *d = desk_head;
    // unsigned int i = 0;

    // while (d != NULL && i < num_clients) {
    //     node_t *n = first_extrema(d->root);
    //     while (n != NULL) {
    //         wins[i++] = n->client->window;
    //         n = next_leaf(n);
    //     }
    //     d = d->next;
    // }

    // if (i != num_clients)
    //     return;

    // xcb_ewmh_set_client_list(ewmh, default_screen, num_clients, wins);
}

