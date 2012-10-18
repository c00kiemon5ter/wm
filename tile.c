#include <stdint.h>

#include "tile.h"
#include "helpers.h"
#include "window.h"

/**
 * Max layout - all windows fullscreen
 *
 * +--------+
 * |        |
 * | M A X  |
 * |        |
 * +--------+
 *
 */
void monocle(const monitor_t *mon, unsigned short wins)
{
    for (client_t *c = cfg.clients; c && wins; c = c->next)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
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
 *
 */
void grid(const monitor_t *mon, unsigned short wins)
{
    const unsigned short n = wins;
    unsigned short cols = 0, rows = 0;
    unsigned short ncol = 0, nrow = 0;

    /* calculate number of columns and rows - emulate square root */
    for (cols = 0; cols <= n/2; cols++)
        if (cols*cols >= n)
            break;
    if (n == 5)
        cols = 2;
    rows = n/cols;

    unsigned short i = 0;

    const uint16_t client_width  = mon->geom.width / (cols ? cols : 1);
    const uint16_t client_height = mon->geom.height;

    for (client_t *c = cfg.clients; c && i < wins; c = c->next)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
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
 *
 * +--------+
 * |    |   |
 * |    +---|
 * |    |   |
 * +--------+
 *
 */
void vstack(const monitor_t *mon, unsigned short wins)
{
    client_t *c = cfg.clients;

    /* place the first 'm_wins' windows in the master area */
    const uint16_t m_area = mon->geom.width * M_AREA_FACT + mon->m_area;
    const uint16_t m_wins = (wins <= mon->m_wins) ? wins - 1 : mon->m_wins;

    for (unsigned short i = 0, m_h = mon->geom.height / m_wins; c && i < m_wins; c = c->next)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c))
            client_set_geom(c, mon->geom.x, mon->geom.y + i++ * m_h, m_area, m_h);

    /* all other windows go to the stack */
    wins -= m_wins;

    const int16_t client_x = mon->geom.x + m_area;
    const uint16_t client_w = mon->geom.width - m_area;
    const uint16_t client_h = mon->geom.height / wins;

    for (int16_t client_y = mon->geom.y; c && wins; c = c->next)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_set_geom(c, client_x, client_y, client_w, client_h);
            client_y += client_h;
            --wins;
        }
}

/**
 * horizontal stack - bottom stack
 *
 * +--------+
 * |        |
 * |--+--+--|
 * |  |  |  |
 * +--------+
 *
 */
void hstack(const monitor_t *mon, unsigned short wins)
{
    client_t *c = cfg.clients;

    /* place the first 'm_wins' windows in the master area */
    const uint16_t m_area = mon->geom.height * M_AREA_FACT + mon->m_area;
    const uint16_t m_wins = (wins <= mon->m_wins) ? wins - 1 : mon->m_wins;

    for (unsigned short i = 0, m_w = mon->geom.width / m_wins; c && i < m_wins; c = c->next)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c))
            client_set_geom(c, mon->geom.x + i++ * m_w, mon->geom.y, m_w, m_area);

    /* all other windows go to the stack */
    wins -= m_wins;

    const int16_t client_y = mon->geom.y + m_area;
    const uint16_t client_w = mon->geom.width / wins;
    const uint16_t client_h = mon->geom.height - m_area;

    for (int16_t client_x = mon->geom.x; c && wins; c = c->next)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_set_geom(c, client_x, client_y, client_w, client_h);
            client_x += client_w;
            --wins;
        }
}

/**
 * tile the given monitor
 * according to its mode
 */
void tile(const monitor_t *mon)
{
    unsigned short num_windows = 0;
    for (client_t *c = cfg.clients; c; c = c->next)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c))
            ++num_windows;

    if (!num_windows)
        return;

    /* if a single window then maximize it */
    if (num_windows == 1) {
        monocle(mon, num_windows);
        return;
    }

    switch (mon->mode) {
        case VSTACK:    vstack(mon, num_windows);   break;
        case HSTACK:    hstack(mon, num_windows);   break;
        case GRID:      grid(mon, num_windows);     break;
        case MONOCLE:   monocle(mon, num_windows);  break;
        case FLOAT:     /* do not handle */         break;
        default: PRINTF("unknown layout: %d\n", mon->mode); break;
    }
    PRINTF("count of windows that match tag '%u' : %u\n", mon->tags, num_windows);
}

