#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "cookiewm.h"
#include "global.h"
#include "helpers.h"
#include "common.h"
#include "screen.h"
#include "ewmh.h"
#include "events.h"
#include "messages.h"

struct configuration cfg = { 0, 0, 0, 0, 0, { { 0 } }, 0, 0, 0, 0 };

/**
 * setup server connection
 * set screen and monitor info
 * set root event mask - check for other wm
 * set ewmh atoms and default wm_name
 */
static void init_xcb(int *dpy_fd)
{
    cfg.conn = xcb_connect((void *)0, &cfg.def_screen);
    if (xcb_connection_has_error(cfg.conn))
        err("connection has errors\n");
    *dpy_fd = xcb_get_file_descriptor(cfg.conn);

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(cfg.conn));
    for (int screen = 0; iter.rem && screen != cfg.def_screen; xcb_screen_next(&iter), screen++);
    cfg.screen = iter.data;

    /* check for randr and xinerama extensions
     * or fallback to zaphod/dual head mode.
     * initialize monitor structs and tags.
     */
    if (!randr() && !xinerama())
        zaphod();

    /* set the event mask for the root window
     * the event mask defines for which events
     * we will be notified about.
     * if setting the event mask fails, then
     * another window manager is running, as
     * XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
     * can be only be set by one client.
     */
    const uint32_t values[] = {ROOT_EVENT_MASK};
    const xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(cfg.conn, cfg.screen->root, XCB_CW_EVENT_MASK, values);
    const xcb_generic_error_t *error = xcb_request_check(cfg.conn, cookie);
    if (error) {
        xcb_disconnect(cfg.conn);
        err("another window manager is already running\n");
    }

    /* set ewmh support */
    ewmh_init();
    ewmh_set_supported_atoms();
    ewmh_update_wm_name(WM_NAME);
}

/**
 * setup socket connection
 */
static void init_socket(int *sock_fd)
{
    struct sockaddr_un sock_address = { 0,  { 0 } };
    char *socket_path = getenv(SOCKET_ENV_VAR);
    if (!socket_path || strlen(socket_path) == 0) {
        warn("environmental variable '%s' is not set or empty - using default value: %s\n", SOCKET_ENV_VAR, DEFAULT_SOCKET_PATH);
        socket_path = DEFAULT_SOCKET_PATH;
    }

    sock_address.sun_family = AF_UNIX;
    size_t n = snprintf(sock_address.sun_path, sizeof(sock_address.sun_path), "%s", socket_path);
    if (n >= sizeof(sock_address.sun_path))
        err("value too long for environmental variable '%s'\n", SOCKET_ENV_VAR);

    unlink(socket_path);
    *sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (*sock_fd == -1)
        err("failed to create socket\n");

    if (bind(*sock_fd, (struct sockaddr *) &sock_address, sizeof(sock_address)) == -1)
        err("failed to bind to socket\n");

    if (listen(*sock_fd, SOMAXCONN) == -1)
        err("failed to listen for connections\n");

}

/**
 * initialize wm structs
 */
void init_wm(void)
{
    for (monitor_t *m = cfg.monitors; m; m = m->next) {
        BIT_SET(m->tags, 0);
        m->m_area = 0;
        m->m_wins = 1;
        m->mode = VSTACK;
    }

    cfg.cur_mon = &cfg.monitors;
    cfg.cur_client = &cfg.clients;

    for (size_t i = 0; i < LENGTH(cfg.tag_names); i++)
        snprintf(cfg.tag_names[i], sizeof(cfg.tag_names[0]), "%zd", i);
}

/**
 * check if a new event arrived
 * and handle the event
 */
static void check_event(const int dpy_fd, const fd_set *fds)
{
    if (FD_ISSET(dpy_fd, fds)) {
        xcb_generic_event_t *event = (void *)0;
        while ((event = xcb_poll_for_event(cfg.conn))) {
            handle_event(event);
            if (event)
                free(event);
        }
    }

    if (xcb_connection_has_error(cfg.conn)) {
        err("connection has errors\n");
    }
}

/**
 * check if a new message arrived and
 * handle the message and the response
 */
static void check_message(const int sock_fd, const fd_set *fds)
{
    char msg[BUFSIZ] = { 0 };
    char rsp[BUFSIZ] = { 0 };

    if (FD_ISSET(sock_fd, fds)) {
        const int ret_fd = accept(sock_fd, (void *)0, (void *)0);
        if (ret_fd == -1) {
            warn("failed to accept connection\n");
            return;
        }

        ssize_t n = recv(ret_fd, msg, sizeof(msg), 0);
        if (n <= 0) {
            warn("failed to receive message\n");
            return;
        }

        msg[n] = '\0';
        process_message(msg, rsp);
        if (send(ret_fd, rsp, strlen(rsp), 0) == -1)
            warn("failed to send response\n");

        if (ret_fd)
            close(ret_fd);
    }
}

static void wait_event_or_message(const int dpy_fd, const int sock_fd)
{
    fd_set fds = { { 0 } };
    const int sel = max(sock_fd, dpy_fd) + 1;

    while (cfg.running) {
        FD_ZERO(&fds);
        FD_SET(sock_fd, &fds);
        FD_SET(dpy_fd, &fds);

        if (select(sel, &fds, (void *)0, (void *)0, (void *)0))  {
            check_message(sock_fd, &fds);
            check_event(dpy_fd, &fds);
        }
    }
}

void quit(void)
{
    cfg.running = false;
}

int main(void)
{
    int dpy_fd = 0, sock_fd = 0;
    cfg.running = true;

    /* initialization */
    init_xcb(&dpy_fd);
    init_socket(&sock_fd);
    init_wm();

    /* main loop */
    wait_event_or_message(dpy_fd, sock_fd);

    /* cleanup */
    xcb_ewmh_connection_wipe(cfg.ewmh);
    if (cfg.ewmh)
        free(cfg.ewmh);

    xcb_flush(cfg.conn);
    xcb_disconnect(cfg.conn);

    if (dpy_fd)
        close(dpy_fd);
    if (sock_fd)
        close(sock_fd);

    return EXIT_SUCCESS;
}

