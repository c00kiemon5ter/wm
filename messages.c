#include <string.h>
#include <stdbool.h>

#include "messages.h"
#include "common.h"
#include "helpers.h"
#include "monitor.h"
#include "client.h"
#include "tile.h"

#define OK_RESPONSE         "ok"
#define INVALID_INPUT       "invalid input"
#define UNKNOWN_COMMAND     "unknown command"

inline static
bool quit(__attribute__((unused)) char *unused)
{
    cfg.running = false;
    return true;
}

static
bool kill(__attribute__((unused)) char *unused)
{
    if (cfg.flist) {
        monitor_t *m = cfg.flist->mon;
        if (!client_kill(cfg.flist))
            tile(m);
    }

    return true;
}

static inline
bool mtile(__attribute__((unused)) char *unused)
{
    tile(cfg.monitors);
    return true;
}

static
bool toggle_floating(__attribute__((unused)) char *unused)
{
    if (cfg.flist) {
        cfg.flist->is_floating = !cfg.flist->is_floating;
        tile(cfg.monitors);
    }

    return true;
}

inline static
bool focus_cnext(__attribute__((unused)) char *unused)
{
    client_focus_next();
    return true;
}

inline static
bool focus_cprev(__attribute__((unused)) char *unused)
{
    client_focus_prev();
    return true;
}

inline static
bool focus_mnext(__attribute__((unused)) char *unused)
{
    monitor_focus_next();
    client_focus_first(cfg.monitors);
    return true;
}

inline static
bool focus_mprev(__attribute__((unused)) char *unused)
{
    monitor_focus_prev();
    client_focus_first(cfg.monitors);
    return true;
}

inline static
bool set_border(char *border)
{
    if (!border)
        return false;

    int status = sscanf(border, "%hu", &cfg.monitors->border);

    if (status == EOF || status == 0)
        return false;

    tile(cfg.monitors);
    return true;
}

inline static
bool set_spacer(char *spacer)
{
    if (!spacer)
        return false;

    int status = sscanf(spacer, "%hu", &cfg.monitors->spacer);

    if (status == EOF || status == 0)
        return false;

    tile(cfg.monitors);
    return true;
}

inline static
bool set_m_wins(char *m_wins)
{
    if (!m_wins)
        return false;

    int status = sscanf(m_wins, "%hu", &cfg.monitors->m_wins);

    if (status == EOF || status == 0)
        return false;

    tile(cfg.monitors);
    return true;
}

static
bool set_layout(char *mode)
{
    if (!mode)
        return false;

    layout_t layout = LAYOUTS;

    if (strcmp(mode, "tile") == 0)
        layout = VSTACK;
    else if (strcmp(mode, "vstack") == 0)
        layout = VSTACK;
    else if (strcmp(mode, "nvstack") == 0)
        layout = VSTACK;

    else if (strcmp(mode, "bstack") == 0)
        layout = HSTACK;
    else if (strcmp(mode, "hstack") == 0)
        layout = HSTACK;
    else if (strcmp(mode, "nhstack") == 0)
        layout = HSTACK;

    else if (strcmp(mode, "grid") == 0)
        layout = GRID;

    else if (strcmp(mode, "max") == 0)
        layout = MONOCLE;
    else if (strcmp(mode, "monocle") == 0)
        layout = MONOCLE;

    else if (strcmp(mode, "float") == 0)
        layout = FLOAT;

    if (layout == LAYOUTS)
        return false;

    if (cfg.monitors->layout != layout)
        tile(cfg.monitors);

    return true;
}

static
bool set_ctag(char *tag)
{
    if (!tag)
        return false;

    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);

    if (status == EOF || status == 0 || ntag > sizeof(uint16_t)*CHAR_BIT)
        return false;

    BIT_FLIP(cfg.flist->tags, ntag);
    return true;
}

static
bool set_mtag(char *tag)
{
    if (!tag)
        return false;

    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);

    if (status == EOF || status == 0 || ntag > sizeof(uint16_t)*CHAR_BIT)
        return false;

    BIT_FLIP(cfg.monitors->tags, ntag);
    tile(cfg.monitors);
    return true;
}

static
bool goto_tag(char *tag)
{
    if (!tag)
        return false;

    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);

    if (status == EOF || status == 0 || ntag > sizeof(uint16_t)*CHAR_BIT)
        return false;

    cfg.monitors->tags = 0;
    BITMASK_SET(cfg.monitors->tags, ntag);
    tile(cfg.monitors);
    client_focus_first(cfg.monitors);
    return true;
}

void process_message(char *msg, char *rsp)
{
    PRINTF("got message: %s\n", msg);

    char *cmd = strtok(msg, TOKEN_SEP);
    bool (*func)(char *) = (void *)0;

    if (!cmd || strlen(cmd) == 0)
        func = (void *)0;
    else if (strcmp(cmd, "quit") == 0)
        func = quit;
    else if (strcmp(cmd, "kill") == 0)
        func = kill;
    else if (strcmp(cmd, "tile") == 0)
        func = mtile;
    else if (strcmp(cmd, "ctag") == 0)
        func = set_ctag;
    else if (strcmp(cmd, "mtag") == 0)
        func = set_mtag;
    else if (strcmp(cmd, "gtag") == 0)
        func = goto_tag;
    else if (strcmp(cmd, "cnext") == 0)
        func = focus_cnext;
    else if (strcmp(cmd, "cprev") == 0)
        func = focus_cprev;
    else if (strcmp(cmd, "mnext") == 0)
        func = focus_mnext;
    else if (strcmp(cmd, "mprev") == 0)
        func = focus_mprev;
    /* ** those expect an argument ** */
    else if (strcmp(cmd, "border") == 0)
        func = set_border;
    else if (strcmp(cmd, "spacer") == 0)
        func = set_spacer;
    else if (strcmp(cmd, "m_wins") == 0)
        func = set_m_wins;
    else if (strcmp(cmd, "layout") == 0)
        func = set_layout;
    else if (strcmp(cmd, "tfloat") == 0)
        func = toggle_floating;

    if (!func)
        snprintf(rsp, BUF_NAME_LEN, "?? %s", UNKNOWN_COMMAND);
    else if (func(strtok((void *)0, TOKEN_SEP)))
        snprintf(rsp, BUF_NAME_LEN, ":: %s", OK_RESPONSE);
    else
        snprintf(rsp, BUF_NAME_LEN, "!! %s", INVALID_INPUT);

    PRINTF("composed response: %s\n", rsp);
}

