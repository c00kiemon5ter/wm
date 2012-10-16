#include <stdint.h>

#include "tile.h"
#include "helpers.h"
#include "window.h"

/**
 * Max layout - all windows fullscreen
 * +--------+
 * |        |
 * | M A X  |
 * |        |
 * +--------+
 */
void monocle(const monitor_t *mon, unsigned int wins)
{
    for (client_t *c = cfg.clients; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom_geom(c, mon->geom);
            --wins;
        }
}

/**
 * Grid layout
 *
 * +--------+
 * |  |  |  |
 * +--+--+--+
 * |  |  |  |
 * +--------+
 */
void grid(const monitor_t *mon, unsigned int wins)
{
    const unsigned int n = wins;
    unsigned int cols = 0, rows = 0;
    unsigned int ncol = 0, nrow = 0;

    /* calculate number of columns and rows - emulate square root */
    for (cols = 0; cols <= n/2; cols++)
        if (cols*cols >= n)
            break;
    if (n == 5)
        cols = 2;
    rows = n/cols;

    unsigned int i = 0;

    const uint16_t client_width  = mon->geom.width / (cols ? cols : 1);
    const uint16_t client_height = mon->geom.height;

    for (client_t *c = cfg.clients; c && i < wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            if (i++/rows + 1 > cols - n%cols)
                rows = n/cols + 1;

            client_set_geom(c, mon->geom.x + client_width  * ncol,
                               mon->geom.y + client_height * nrow / rows,
                               client_width, client_height / rows);

            /* all rows filled - reset row count and go to next column */
            if (++nrow >= rows) {
                nrow = 0;
                ncol++;
            }
        }
}

/**
 * vertical stack - the common tiling mode
 * +--------+
 * |    |   |
 * |    +---|
 * |    |   |
 * +--------+
 */
void vstack(const monitor_t *mon, unsigned int wins)
{
    client_t *c = cfg.clients;
    const uint16_t m_area = mon->geom.width * M_AREA_FACT + mon->m_area;

    /* look for the first window */
    while (c && (c->is_floating || c->is_fullscrn || !BITMASK_CHECK(mon->tags, c->tags)))
        c = c->next;

    /* place the first window in the master area */
    client_set_geom(c, mon->geom.x, mon->geom.y, m_area, mon->geom.height);
    --wins;

    const int16_t client_x = mon->geom.x + m_area;
    int16_t client_y = mon->geom.y;
    const uint16_t client_w = mon->geom.width - m_area;
    const uint16_t client_h = mon->geom.height / wins;

    /* all other windows go to the stack */
    for (c = c->next; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom(c, client_x, client_y, client_w, client_h);
            client_y += client_h;
            --wins;
        }
}

/**
 * horizontal stack - bottom stack
 * +--------+
 * |        |
 * |--+--+--|
 * |  |  |  |
 * +--------+
 */
void hstack(const monitor_t *mon, unsigned int wins)
{
    client_t *c = cfg.clients;
    const uint16_t m_area = mon->geom.height * M_AREA_FACT + mon->m_area;

    /* look for the first window */
    while (c && (c->is_floating || c->is_fullscrn || !BITMASK_CHECK(mon->tags, c->tags)))
        c = c->next;

    /* place the first window in the master area */
    client_set_geom(c, mon->geom.x, mon->geom.y, mon->geom.width, m_area);
    --wins;

    int16_t client_x = mon->geom.x;
    const int16_t client_y = mon->geom.y + m_area;
    const uint16_t client_w = mon->geom.width / wins;
    const uint16_t client_h = mon->geom.height - m_area;

    /* all other windows go to the stack */
    for (c = c->next; c && wins; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags)) {
            client_set_geom(c, client_x, client_y, client_w, client_h);
            client_x += client_w;
            --wins;
        }
}

/**
 * tile the given monitor with the given layout
 */
void tile(const monitor_t *mon, layout_t layout)
{
    unsigned int num_windows = 0;
    for (client_t *c = cfg.clients; c; c = c->next)
        if (!c->is_floating && !c->is_fullscrn && BITMASK_CHECK(mon->tags, c->tags))
            ++num_windows;

    if (!num_windows)
        return;

    /* if a single window then maximize it */
    if (num_windows == 1) {
        monocle(mon, num_windows);
        return;
    }

    switch (layout) {
        case VSTACK:    vstack(mon, num_windows);   break;
        case HSTACK:    hstack(mon, num_windows);   break;
        case GRID:      grid(mon, num_windows);     break;
        case MONOCLE:   monocle(mon, num_windows);  break;
        case FLOAT:     /* do not handle */         break;
        default: PRINTF("unknown layout: %d\n", layout); break;
    }
    PRINTF("count of windows that match tag '%u' : %u\n", mon->tags, num_windows);
}

