#include <string.h>

#include "messages.h"
#include "cookiewm.h"
#include "helpers.h"
#include "window.h"
#include "tile.h"

void process_message(char *msg, char *rsp)
{
    PRINTF("got message: %s\n", msg);

    if (strcmp(msg, "quit") == 0) {
        PRINTF("quit message: %s\n", msg);
        quit();
    }

    if (strcmp(msg, "kill") == 0) {
        PRINTF("kill message: %s\n", msg);

        if (!cfg.cur_client)
            return;

        monitor_t *m = cfg.cur_client->mon;
        if (!client_kill(cfg.cur_client))
            tile(m);

        cfg.cur_client = cfg.clients;

        PRINTF("cur client is: %u\n", cfg.cur_client->win);
    }

    if (strcmp(msg, "tile") == 0) {
        PRINTF("tile message %s\n", msg);

        tile(cfg.cur_mon);
    }

    PRINTF("send response: %s\n", rsp);
}

