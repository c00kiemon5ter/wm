#include <string.h>

#include "messages.h"
#include "common.h"
#include "helpers.h"
#include "monitor.h"
#include "client.h"
#include "tile.h"

inline static
void quit(void)
{
    cfg.running = false;
}

static
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

inline static
void set_border(char *border)
{
    int status = sscanf(border, "%hu", &cfg.monitors->border);
    if (status != EOF && status != 0)
        tile(cfg.monitors);
}

inline static
void set_spacer(char *spacer)
{
    int status = sscanf(spacer, "%hu", &cfg.monitors->spacer);
    if (status != EOF && status != 0)
        tile(cfg.monitors);
}

inline static
void set_m_wins(char *m_wins)
{
    int status = sscanf(m_wins, "%hu", &cfg.monitors->m_wins);
    if (status != EOF && status != 0)
        tile(cfg.monitors);
}

static
void set_layout(char *layout)
{
    if (strcmp(layout, "tile") == 0)
        cfg.monitors->layout = VSTACK;
    else if (strcmp(layout, "vstack") == 0)
        cfg.monitors->layout = VSTACK;
    else if (strcmp(layout, "nvstack") == 0)
        cfg.monitors->layout = VSTACK;

    else if (strcmp(layout, "bstack") == 0)
        cfg.monitors->layout = HSTACK;
    else if (strcmp(layout, "hstack") == 0)
        cfg.monitors->layout = HSTACK;
    else if (strcmp(layout, "nhstack") == 0)
        cfg.monitors->layout = HSTACK;

    else if (strcmp(layout, "grid") == 0)
        cfg.monitors->layout = GRID;

    else if (strcmp(layout, "max") == 0)
        cfg.monitors->layout = MONOCLE;
    else if (strcmp(layout, "monocle") == 0)
        cfg.monitors->layout = MONOCLE;

    else
        warn("invalid layout: %s\n", layout);
    tile(cfg.monitors);
}

static
void set_ctag(char *tag)
{
    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);
    if (status != EOF && status != 0)
        BIT_FLIP(cfg.flist->tags, ntag);
}

static
void set_mtag(char *tag)
{
    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);
    if (status != EOF && status != 0) {
        BIT_FLIP(cfg.monitors->tags, ntag);
        tile(cfg.monitors);
    }
}

static
void goto_tag(char *tag)
{
    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);
    if (status != EOF && status != 0) {
        cfg.monitors->tags = 0;
        BITMASK_SET(cfg.monitors->tags, ntag);
        tile(cfg.monitors);
        client_focus_first(cfg.monitors);
    }
}

void process_message(char *msg, char *rsp)
{
    PRINTF("got message: %s\n", msg);

    char *cmd = strtok(msg, TOKEN_SEP);

    if (!cmd || strlen(cmd) == 0)
        return;
    else if (strcmp(cmd, "quit") == 0)
        quit();
    else if (strcmp(cmd, "kill") == 0)
        kill();
    else if (strcmp(cmd, "tile") == 0)
        tile(cfg.monitors);
    else if (strcmp(cmd, "cnext") == 0)
        focus_cnext();
    else if (strcmp(cmd, "cprev") == 0)
        focus_cprev();
    else if (strcmp(cmd, "mnext") == 0)
        focus_mnext();
    else if (strcmp(cmd, "mprev") == 0)
        focus_mprev();
    else if (strcmp(cmd, "border") == 0)
        set_border(strtok(NULL, TOKEN_SEP));
    else if (strcmp(cmd, "spacer") == 0)
        set_spacer(strtok(NULL, TOKEN_SEP));
    else if (strcmp(cmd, "m_wins") == 0)
        set_m_wins(strtok(NULL, TOKEN_SEP));
    else if (strcmp(cmd, "layout") == 0)
        set_layout(strtok(NULL, TOKEN_SEP));
    else if (strcmp(cmd, "ctag") == 0)
        set_ctag(strtok(NULL, TOKEN_SEP));
    else if (strcmp(cmd, "mtag") == 0)
        set_mtag(strtok(NULL, TOKEN_SEP));
    else if (strcmp(cmd, "gtag") == 0)
        goto_tag(strtok(NULL, TOKEN_SEP));
    else
        warn("unknown command: %s\n", cmd);

    PRINTF("composed response: %s\n", rsp);
}

