#include <stdint.h>

#include "tile.h"
#include "helpers.h"
#include "window.h"

void monocle(const monitor_t *mon, unsigned int wins)
{
    for (client_t *c = cfg.clients; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom_geom(c, mon->geom);
            --wins;
        }
}

void grid(const monitor_t *mon, unsigned int wins)
{
    const int n = wins;
    int cols = 0, col_num = 0, row_num = 0, i = -1;

    for (cols = 0; cols <= n/2; cols++)
        if (cols*cols >= n)
            break;

    if (n == 5)
        cols = 2;

    int rows = n/cols;
    uint16_t client_width  = mon->geom.width / (cols ? cols : 1);
    uint16_t client_height = mon->geom.height;

    for (client_t *c = cfg.clients; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            ++i;
            if (i/rows + 1 > cols - n%cols)
                rows = n/cols + 1;
            client_set_geom(c, mon->geom.x + col_num*client_width,
                               mon->geom.y + row_num*client_height/rows,
                               client_width, client_height/rows);
            if (++row_num >= rows) {
                row_num = 0;
                col_num++;
            }
            --wins;
        }
}

void vstack(const monitor_t *mon, unsigned int wins)
{
    client_t *c = cfg.clients;
    uint16_t m_area = mon->geom.width * 0.6;

    for (; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom(c, mon->geom.x, mon->geom.y, m_area, mon->geom.height);
            --wins;
            break;
        }

    if (!c)
        return;

    const int16_t client_x = mon->geom.x + m_area;
    int16_t client_y = mon->geom.y;
    const uint16_t client_w = mon->geom.width - m_area;
    const uint16_t client_h = mon->geom.height / wins;
    for (c = c->next; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom(c, client_x, client_y, client_w, client_h);
            client_y += client_h;
            --wins;
        }
}

void bstack(const monitor_t *mon, unsigned int wins)
{
    client_t *c = cfg.clients;
    uint16_t m_area = mon->geom.height * 0.6;

    for (; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom(c, mon->geom.x, mon->geom.y, mon->geom.width, m_area);
            --wins;
            break;
        }

    if (!c)
        return;

    int16_t client_x = mon->geom.x;
    const int16_t client_y = mon->geom.y + m_area;
    const uint16_t client_w = mon->geom.width / wins;
    const uint16_t client_h = mon->geom.height - m_area;
    for (c = c->next; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom(c, client_x, client_y, client_w, client_h);
            client_x += client_w;
            --wins;
        }
}

void tile(const monitor_t *mon, layout_t layout)
{
    unsigned int num_windows = 0;
    for (client_t *c = cfg.clients; c; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags))
            ++num_windows;

    if (!num_windows)
        return;

    if (num_windows == 1) {
        monocle(mon, num_windows);
        return;
    }

    switch (layout) {
        case VSTACK:    vstack(mon, num_windows);   break;
        case BSTACK:    bstack(mon, num_windows);   break;
        case GRID:      grid(mon, num_windows);     break;
        case MAX:       monocle(mon, num_windows);  break;
        case FLOAT:     /* do not handle */         break;
        default: PRINTF("unknown layout: %d\n", layout); break;
    }
    PRINTF("count of windows that match tag '%u' : %u\n", mon->tags, num_windows);
}

