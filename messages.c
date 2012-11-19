#include <string.h>
#include <stdbool.h>

#include "messages.h"
#include "common.h"
#include "helpers.h"
#include "monitor.h"
#include "client.h"
#include "tile.h"

#define OK_RESPONSE         ":) ok"
#define INVALID_INPUT       ":( invalid input"
#define UNKNOWN_COMMAND     ":S unknown command"

inline static
bool quit(__attribute__((unused)) char *unused)
{
    cfg.running = false;
    return true;
}

static
bool kill_client(__attribute__((unused)) char *unused)
{
    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    monitor_t *m = cfg.flist->mon;
    if (!client_kill(cfg.flist))
        tile(m);

    return true;
}

inline static
bool tile_monitor(__attribute__((unused)) char *unused)
{
    tile(cfg.monitors);
    return true;
}

static
bool tag_client(char *tag)
{
    if (!tag)
        return false;

    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);

    if (status == EOF || status == 0 || ntag >= LENGTH(cfg.tag_names))
        return false;

    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    BIT_FLIP(cfg.flist->tags, ntag);

    if (!IS_VISIBLE(cfg.flist)) {
        tile(cfg.monitors);
        client_hide(cfg.flist);
        client_focus(client_fnext(cfg.flist, cfg.monitors));
    }

    /* FIXME ctag -- what if there is no set tag left ? how do we access the window again ? */
    // if (!cfg.flist->tags)
    //     cfg.flist->tags = ??;

    return true;
}

static
bool tag_monitor(char *tag)
{
    if (!tag)
        return false;

    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);

    if (status == EOF || status == 0 || ntag >= LENGTH(cfg.tag_names))
        return false;

    BIT_FLIP(cfg.monitors->tags, ntag);

    /* TODO:
     * loop through mon clients
     * hide non-visible clients
     * show visible clients */
    // FIXME: show_hide(cfg.monitors);
    /* TODO:
     * loop though mon clients
     * and fix stacking order */
    // FIXME: restack(cfg.monitors);
    tile(cfg.monitors);

    return true;
}

static
bool view_tag(char *tag)
{
    if (!tag)
        return false;

    uint16_t ntag = 0;
    int status = sscanf(tag, "%hu", &ntag);

    if (status == EOF || status == 0 || ntag >= LENGTH(cfg.tag_names))
        return false;

    cfg.monitors->tags = 0;
    BIT_SET(cfg.monitors->tags, ntag);

    // FIXME: should show_hide(..)
    tile(cfg.monitors);

    client_t *c = cfg.flist;
    if (c && (!IS_VISIBLE(c) || !ON_MONITOR(cfg.monitors, c)))
        c = client_fnext(c, cfg.monitors);

    client_focus(c);

    return true;
}

static
bool hide_all(__attribute__((unused)) char *unused)
{
    cfg.monitors->tags = 0;
    // FIXME: should show_hide(..)

    return true;
}

static
bool show_all(__attribute__((unused)) char *unused)
{
    cfg.monitors->tags = -1;
    tile(cfg.monitors);
    // FIXME: should show_hide(..)

    return true;
}

static
bool move_down(__attribute__((unused)) char *unused)
{
    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    client_t *n = client_vnext(cfg.flist, cfg.monitors);

    /* f: first , c: current , n: next , l: last
     *
     * a) the current client is before the next one,
     *    so it must be placed after the next one
     * [f]->..->[c]->..->[n]->..->[l]->  ===>  [f]->..->[n]->..->[c]->..->[l]->
     *
     * b) the current client is after the next one,
     *    because it is the last client on the screen,
     *    so it must be placed on the head of the list
     * [f]->..->[n]->..->[c]->..->[l]->  ===>  [c]->[f]->..->[n]->..->[l]->
     */
    bool n_after_c = false;
    for (client_t *t = cfg.flist->vnext; t && !(n_after_c = t == n); t = t->vnext);

    if (n_after_c)
        client_move_after(cfg.flist, n);
    else {
        /* FIXME use client_move_before(cfg.flist, n); when it works */
        client_vunlink(cfg.flist);
        client_link_head(cfg.flist);
    }

    tile(cfg.monitors);

    return true;
}

static
bool move_up(__attribute__((unused)) char *unused)
{
    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    client_t *p = client_vprev(cfg.flist, cfg.monitors);

    /* f: first , p: prev , c: current , l: last
     *
     * a) the current client is after the prev one,
     *    so it must be placed before the prev one
     * [f]->..->[p]->..->[c]->..->[l]->  ===>  [f]->..->[c]->..->[p]->..->[l]->
     *
     * b) the current client is before the prev one,
     *    because it is the first client on the screen,
     *    so it must be placed on the tail of the list
     * [f]->..->[c]->..->[p]->..->[l]->  ===>  [f]->..->[p]->..->[l]->[c]->
     */
    bool p_before_c = false;
    for (client_t *t = p->vnext; t && !(p_before_c = t == cfg.flist); t = t->vnext);

    if (p_before_c)
        client_move_before(cfg.flist, p);
    else
        client_move_after(cfg.flist, p);

    tile(cfg.monitors);

    return true;
}

static
bool sticky(__attribute__((unused)) char *unused)
{
    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    cfg.flist->tags = -1;

    return true;
}

static
bool focus_next_client(__attribute__((unused)) char *unused)
{
    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    client_t *c = client_vnext(cfg.flist, cfg.flist->mon);
    client_focus(c);

    return true;
}

static
bool focus_prev_client(__attribute__((unused)) char *unused)
{
    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    client_t *c = client_vprev(cfg.flist, cfg.flist->mon);
    client_focus(c);

    return true;
}

static
bool focus_next_monitor(__attribute__((unused)) char *unused)
{
    monitor_t *m = monitor_next(cfg.monitors);
    monitor_focus(m);

    client_t *c = cfg.flist;
    if (c && (!IS_VISIBLE(c) || !ON_MONITOR(m, c)))
        c = client_fnext(c, m);

    client_focus(c);

    return true;
}

static
bool focus_prev_monitor(__attribute__((unused)) char *unused)
{
    monitor_t *m = monitor_prev(cfg.monitors);
    monitor_focus(m);

    client_t *c = cfg.flist;
    if (c && (!IS_VISIBLE(c) || !ON_MONITOR(m, c)))
        c = client_fnext(c, m);

    client_focus(c);

    return true;
}

static
bool set_border(char *border)
{
    if (!border)
        return false;

    int status = sscanf(border, "%hu", &cfg.monitors->border);

    if (status == EOF || status == 0)
        return false;

    // FIXME: should set_border(..)
    tile(cfg.monitors);

    return true;
}

static
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

static
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

    /* side stack layout */
    if (strcmp(mode, "tile") == 0)
        layout = VSTACK;
    else if (strcmp(mode, "vstack") == 0)
        layout = VSTACK;
    else if (strcmp(mode, "nvstack") == 0)
        layout = VSTACK;
    /* bottom stack layout */
    else if (strcmp(mode, "bstack") == 0)
        layout = HSTACK;
    else if (strcmp(mode, "hstack") == 0)
        layout = HSTACK;
    else if (strcmp(mode, "nhstack") == 0)
        layout = HSTACK;
    /* monocle / max layout */
    else if (strcmp(mode, "max") == 0)
        layout = MONOCLE;
    else if (strcmp(mode, "monocle") == 0)
        layout = MONOCLE;
    /* grid layout */
    else if (strcmp(mode, "grid") == 0)
        layout = GRID;
    /* float layout */
    else if (strcmp(mode, "float") == 0)
        layout = FLOAT;

    if (layout == LAYOUTS)
        return false;

    if (cfg.monitors->layout != layout) {
        cfg.monitors->layout = layout;
        tile(cfg.monitors);
    }

    return true;
}

static
bool toggle_floating(__attribute__((unused)) char *unused)
{
    if (!cfg.flist || !IS_VISIBLE(cfg.flist) || !ON_MONITOR(cfg.monitors, cfg.flist))
        return false;

    cfg.flist->is_floating = !cfg.flist->is_floating;
    tile(cfg.monitors);

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
        func = kill_client;
    else if (strcmp(cmd, "tile") == 0)
        func = tile_monitor;
    else if (strcmp(cmd, "tagc") == 0)
        func = tag_client;
    else if (strcmp(cmd, "tagm") == 0)
        func = tag_monitor;
    else if (strcmp(cmd, "vtag") == 0)
        func = view_tag;
    else if (strcmp(cmd, "hideall") == 0)
        func = hide_all;
    else if (strcmp(cmd, "showall") == 0)
        func = show_all;
    else if (strcmp(cmd, "movedn") == 0)
        func = move_down;
    else if (strcmp(cmd, "moveup") == 0)
        func = move_up;
    else if (strcmp(cmd, "sticky") == 0)
        func = sticky;
    else if (strcmp(cmd, "nextc") == 0)
        func = focus_next_client;
    else if (strcmp(cmd, "prevc") == 0)
        func = focus_prev_client;
    else if (strcmp(cmd, "nextm") == 0)
        func = focus_next_monitor;
    else if (strcmp(cmd, "prevm") == 0)
        func = focus_prev_monitor;
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
        snprintf(rsp, BUF_NAME_LEN, "%s", UNKNOWN_COMMAND);
    else if (func(strtok((void *)0, TOKEN_SEP)))
        snprintf(rsp, BUF_NAME_LEN, "%s", OK_RESPONSE);
    else
        snprintf(rsp, BUF_NAME_LEN, "%s", INVALID_INPUT);

    PRINTF("composed response: %s\n", rsp);
}

