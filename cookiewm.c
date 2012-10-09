#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

configuration cfg;

/**
 * setup server connection
 * set screen and monitor info
 * set root event mask - check for other wm
 * set ewmh atoms and default wm_name
 */
static void init_xcb(int *dpy_fd)
{
    cfg.connection = xcb_connect(NULL, &cfg.default_screen);
    if (xcb_connection_has_error(cfg.connection))
        err("connection has errors\n");
    *dpy_fd = xcb_get_file_descriptor(cfg.connection);

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(cfg.connection));
    for (int screen = 0; screen < cfg.default_screen; screen++, xcb_screen_next(&iter));
    cfg.screen = iter.data;

    /* check for randr and xinerama extensions
     * if not available try the old dual head.
     * will initialize monitor structs.
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
    uint32_t values[] = {ROOT_EVENT_MASK};
    xcb_void_cookie_t   cookie = xcb_change_window_attributes_checked(cfg.connection, cfg.screen->root, XCB_CW_EVENT_MASK, values);
    xcb_generic_error_t *error = xcb_request_check(cfg.connection, cookie);
    if (error != NULL) {
        xcb_disconnect(cfg.connection);
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
    struct sockaddr_un sock_address;
    char *socket_path = getenv(SOCKET_ENV_VAR);
    if (socket_path == NULL || strlen(socket_path) == 0) {
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
 * check if a new event arrived
 * and handle the event
 */
static void check_event(int dpy_fd, fd_set *fds)
{
    if (FD_ISSET(dpy_fd, fds)) {
        xcb_generic_event_t *event;
        while ((event = xcb_poll_for_event(cfg.connection))) {
            handle_event(event);
            if (event)
                free(event);
        }
    }

    if (xcb_connection_has_error(cfg.connection)) {
        err("connection has errors\n");
    }
}

/**
 * check if a new message arrived and
 * handle the message and the response
 */
static void check_message(int sock_fd, fd_set *fds)
{
    char msg[BUFSIZ];
    char rsp[BUFSIZ];

    if (FD_ISSET(sock_fd, fds)) {
        int ret_fd = accept(sock_fd, NULL, 0);
        if (ret_fd == -1) {
            warn("failed to accept connection\n");
            return;
        }

        ssize_t n = recv(ret_fd, msg, sizeof(msg), 0);
        if (n <= 0) {
            warn("failed to receive message\n");
            return;
        }

        msg[n] = 0;
        process_message(msg, rsp);
        if (send(ret_fd, rsp, strlen(rsp), 0) == -1)
            warn("failed to send response\n");

        if (ret_fd)
            close(ret_fd);
    }
}

static void wait_event_or_message(int dpy_fd, int sock_fd)
{
    fd_set fds;
    int sel = MAX(sock_fd, dpy_fd) + 1;
    cfg.running = true;

    while (cfg.running) {
        FD_ZERO(&fds);
        FD_SET(sock_fd, &fds);
        FD_SET(dpy_fd, &fds);

        if (select(sel, &fds, NULL, NULL, NULL))  {
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
    int dpy_fd, sock_fd;

    /* initialization */
    init_xcb(&dpy_fd);
    init_socket(&sock_fd);

    /* main loop */
    wait_event_or_message(dpy_fd, sock_fd);

    /* cleanup */
    xcb_ewmh_connection_wipe(cfg.ewmh);
    if (cfg.ewmh)
        free(cfg.ewmh);

    xcb_flush(cfg.connection);
    xcb_disconnect(cfg.connection);

    if (dpy_fd)
        close(dpy_fd);
    if (sock_fd)
        close(sock_fd);

    return EXIT_SUCCESS;
}

