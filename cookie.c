#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "helpers.h"
#include "common.h"

int main(int argc, char *argv[])
{
    int sock_fd = 0;
    struct sockaddr_un sock_address = { 0, { 0 } };
    size_t msglen = 0;
    char msg[BUFSIZ] = { 0 };
    char rsp[BUFSIZ] = { 0 };

    if (argc < 2)
        err("no arguments given\n");

    char *socket_path = getenv(SOCKET_ENV_VAR);
    if (!socket_path || strlen(socket_path) == 0) {
        warn("environmental variable '%s' is not set or empty - using default value: %s\n", SOCKET_ENV_VAR, DEFAULT_SOCKET_PATH);
        socket_path = DEFAULT_SOCKET_PATH;
    }

    sock_address.sun_family = AF_UNIX;
    size_t n = snprintf(sock_address.sun_path, sizeof(sock_address.sun_path), "%s", socket_path);
    if (n >= sizeof(sock_address.sun_path))
        err("value too long for environmental variable '%s'\n", SOCKET_ENV_VAR);

    for (size_t offset = 0, len = sizeof(msg), n = 0; --argc && ++argv && len > 0; offset += n, len -= n)
        n = snprintf(msg + offset, len, "%s ", *argv);

    msglen = strlen(msg);
    if (msg[msglen - 1] == ' ')
        msg[--msglen] = '\0';

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1)
        err("failed to create socket\n");

    if (connect(sock_fd, (struct sockaddr *) &sock_address, sizeof(sock_address)) == -1)
        err("failed to connect to socket\n");

    if (send(sock_fd, msg, msglen, 0) == -1)
        err("failed to send data\n");

    ssize_t rsplen = recv(sock_fd, rsp, sizeof(rsp), 0);
    if (rsplen == -1) {
        err("failed to get response\n");
    } else if (rsplen > 0) {
        rsp[rsplen] = '\0';
        printf("%s\n", rsp);
    }

    if (sock_fd)
        close(sock_fd);

    return EXIT_SUCCESS;
}

