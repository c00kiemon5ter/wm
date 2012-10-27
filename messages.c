#include <string.h>

#include "messages.h"
#include "helpers.h"
#include "monitor.h"
#include "client.h"
#include "tile.h"

inline static
void quit(void)
{
    cfg.running = false;
}

void kill(void)
{
    if (!cfg.flist)
        return;

    monitor_t *m = cfg.flist->mon;
    if (!client_kill(cfg.flist))
        tile(m);
}

inline static
void focus_cnext(void)
{
    client_focus_next();
}

inline static
void focus_cprev(void)
{
    client_focus_prev();
}

inline static
void focus_mnext(void)
{
    monitor_focus_next();
    client_focus_first(cfg.monitors);
}

inline static
void focus_mprev(void)
{
    monitor_focus_prev();
    client_focus_first(cfg.monitors);
}

void process_message(char *msg, char *rsp)
{
    PRINTF("got message: %s\n", msg);

    if (!msg || strlen(msg) == 0)
        return;
    else if (strcmp(msg, "quit") == 0)
        quit();
    else if (strcmp(msg, "kill") == 0)
        kill();
    else if (strcmp(msg, "tile") == 0)
        tile(cfg.monitors);
    else if (strcmp(msg, "cnext") == 0)
        focus_cnext();
    else if (strcmp(msg, "cprev") == 0)
        focus_cprev();
    else if (strcmp(msg, "mnext") == 0)
        focus_mnext();
    else if (strcmp(msg, "mprev") == 0)
        focus_mprev();
    else
        warn("unknown message: %s\n", msg);
}

