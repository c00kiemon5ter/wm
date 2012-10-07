#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifdef XINERAMA
#include <xcb/xinerama.h>
#endif

#include "cookiewm.h"
#include "global.h"
#include "helpers.h"
#include "common.h"
#include "events.h"
#include "messages.h"

configuration cfg;

void setup(void)
{
    /* TODO */
}

void register_events(void)
{
    /* TODO */
}

void quit(void)
{
    cfg.running = false;
}

int main(void)
{
    fd_set fds;
    int default_screen;
    int dpy_fd, sock_fd;
    struct sockaddr_un sock_address;
    char msg[BUFSIZ];
    char rsp[BUFSIZ];

    /* setup socket connection */
    char *socket_path = getenv(SOCKET_ENV_VAR);
    if (socket_path == NULL || strlen(socket_path) == 0)
        warn("environmental variable '%s' is not set or empty - using default value: %s\n", SOCKET_ENV_VAR, DEFAULT_SOCKET_PATH);

    sock_address.sun_family = AF_UNIX;
    size_t n = snprintf(sock_address.sun_path, sizeof(sock_address.sun_path), "%s", socket_path);
    if (n >= sizeof(sock_address.sun_path))
        err("value too long for environmental variable '%s'\n", SOCKET_ENV_VAR);
    unlink(socket_path);

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1)
        err("failed to create socket\n");

    if (bind(sock_fd, (struct sockaddr *) &sock_address, sizeof(sock_address)) == -1)
        err("failed to bind to socket\n");

    if (listen(sock_fd, SOMAXCONN) == -1)
        err("failed to listen for connections\n");

    /* setup server connection */
    cfg.connection = xcb_connect(NULL, &default_screen);
    if (xcb_connection_has_error(cfg.connection))
        err("failed to connect to server\n");
    dpy_fd = xcb_get_file_descriptor(cfg.connection);

    setup();
    register_events();
    /* TODO implement in the right place */
    //ewmh_update_wm_name();
    //update_root_geom();

    int sel = MAX(sock_fd, dpy_fd) + 1;
    cfg.running = true;
    xcb_generic_event_t *event;

    while (cfg.running) {
        xcb_flush(cfg.connection);

        FD_ZERO(&fds);
        FD_SET(sock_fd, &fds);
        FD_SET(dpy_fd, &fds);

        if (select(sel, &fds, NULL, NULL, NULL))  {
            /* new message arrived */
            if (FD_ISSET(sock_fd, &fds)) {
                int ret_fd = accept(sock_fd, NULL, 0);
                if (ret_fd == -1) {
                    warn("failed to accept connection\n");
                } else if (recv(ret_fd, msg, sizeof(msg), 0) > 0) {
                    msg[n] = 0;
                    process_message(msg, rsp);
                    size_t rsplen = strlen(rsp);
                    if (send(ret_fd, rsp, rsplen, 0) == -1)
                        warn("failed to send response\n");
                    close(ret_fd);
                }
            }

            /* new event arrived */
            if (FD_ISSET(dpy_fd, &fds)) {
                while ((event = xcb_poll_for_event(cfg.connection)) != NULL) {
                    handle_event(event);
                    free(event);
                }
            }
        }

        if (xcb_connection_has_error(cfg.connection))
            err("connection has errors\n");
    }

    close(sock_fd);

    xcb_ewmh_connection_wipe(cfg.ewmh);
    free(cfg.ewmh);

    xcb_flush(cfg.connection);
    xcb_disconnect(cfg.connection);

    return EXIT_SUCCESS;
}

