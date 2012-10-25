#include <string.h>

#include "messages.h"
#include "helpers.h"
#include "window.h"
#include "tile.h"

void quit(void)
{
    cfg.running = false;
}

void process_message(char *msg, char *rsp)
{
    PRINTF("got message: %s\n", msg);

    if (strcmp(msg, "quit") == 0) {
        PRINTF("quit message: %s\n", msg);
        quit();
    }

    if (strcmp(msg, "kill") == 0) {
        PRINTF("kill message: %s\n", msg);

        if (!cfg.client_cur)
            return;

        monitor_t *m = cfg.client_cur->mon;
        if (!client_kill(cfg.client_cur))
            tile(m);

        cfg.client_cur = cfg.clients;

        PRINTF("cur client is: %u\n", cfg.client_cur->win);
    }

    if (strcmp(msg, "tile") == 0) {
        PRINTF("tile message %s\n", msg);

        tile(cfg.monitor_cur);
    }

    PRINTF("send response: %s\n", rsp);
}

