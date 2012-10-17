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

        if (!*cfg.cur_client)
            return;

        monitor_t *m = (*cfg.cur_client)->mon;
        if (!client_kill(*cfg.cur_client))
            tile(m, m->mode);
    }

    PRINTF("send response: %s\n", rsp);
}

