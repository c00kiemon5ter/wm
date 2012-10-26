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

        if (!cfg.flist)
            return;

        monitor_t *m = cfg.flist->mon;
        if (!client_kill(cfg.flist))
            tile(m);

        PRINTF("focused client is: %u\n", cfg.flist->win);
    }

    if (strcmp(msg, "tile") == 0) {
        PRINTF("tile message %s\n", msg);

        tile(cfg.monitors);
    }

    PRINTF("send response: %s\n", rsp);
}

