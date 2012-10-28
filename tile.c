#include <stdint.h>

#include "tile.h"
#include "helpers.h"
#include "client.h"

/* default master area ratio */
#define M_AREA_FACT     0.55
#define OFFSET          (mon->border + mon->spacer)

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
    const xcb_rectangle_t r = {
        .x = mon->geom.x + mon->spacer,
        .y = mon->geom.y + mon->spacer,
        .width  = mon->geom.width  - 2 * OFFSET,
        .height = mon->geom.height - 2 * OFFSET,
    };

    for (client_t *c = cfg.vlist; c && wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_move_resize_geom(c, r);
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

    for (client_t *c = cfg.vlist; c && i < wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            if (i++/rows + 1 > cols - n%cols)
                rows = n/cols + 1;

            client_move_resize(c, mon->geom.x + client_width  * ncol,
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
 * vertical stack - the common tiling layout
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
    client_t *c = cfg.vlist;

    const uint16_t m_area = mon->geom.width * M_AREA_FACT + mon->m_area;
    const uint16_t m_wins = (wins <= mon->m_wins) ? wins - 1 : mon->m_wins;

    xcb_rectangle_t r = {
        .x = mon->geom.x + mon->spacer,
        .y = mon->geom.y + mon->spacer,
        .width  = m_area - OFFSET - mon->border,
        .height = mon->geom.height / m_wins - 2 * OFFSET,
    };

    /* place the first 'm_wins' windows in the master area */
    for (unsigned short i = 0; c && i < m_wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c))
            client_move_resize(c, r.x, r.y + i++ * r.height, r.width, r.height);

    /* all other windows go to the stack */
    wins -= m_wins;

    /* single border in middle */
    uint16_t fix = !mon->spacer * mon->border;

    r.x     += m_area - fix;
    r.width  = mon->geom.width - m_area - 2 * OFFSET + fix;
    r.height = mon->geom.height / wins - OFFSET;

    for (fix = OFFSET; c && wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_move_resize(c, r.x, r.y, r.width, r.height - fix);
            r.y += r.height + fix;
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
    client_t *c = cfg.vlist;

    /* place the first 'm_wins' windows in the master area */
    const uint16_t m_area = mon->geom.height * M_AREA_FACT + mon->m_area;
    const uint16_t m_wins = (wins <= mon->m_wins) ? wins - 1 : mon->m_wins;

    for (unsigned short i = 0, m_w = mon->geom.width / m_wins; c && i < m_wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c))
            client_move_resize(c, mon->geom.x + i++ * m_w, mon->geom.y, m_w, m_area);

    /* all other windows go to the stack */
    wins -= m_wins;

    const int16_t client_y = mon->geom.y + m_area;
    const uint16_t client_w = mon->geom.width / wins;
    const uint16_t client_h = mon->geom.height - m_area;

    for (int16_t client_x = mon->geom.x; c && wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_move_resize(c, client_x, client_y, client_w, client_h);
            client_x += client_w;
            --wins;
        }
}

/**
 * tile the given monitor
 * according to its layout
 */
void tile(const monitor_t *mon)
{
    unsigned short num_windows = 0;
    for (client_t *c = cfg.vlist; c; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c)) {
            client_update_border(c);
            if (IS_TILED(c)) {
                ++num_windows;
            }
        }

    if (!num_windows)
        return;

    PRINTF("found '%u' windows to tile on monitor\n", num_windows);

    /* if a single window then maximize it */
    if (num_windows == 1) {
        monocle(mon, num_windows);
        return;
    }

    switch (mon->layout) {
        case VSTACK:    vstack(mon, num_windows);   break;
        case HSTACK:    hstack(mon, num_windows);   break;
        case GRID:      grid(mon, num_windows);     break;
        case MONOCLE:   monocle(mon, num_windows);  break;
        case FLOAT:     /* do not handle */         break;
        default: PRINTF("unknown layout: %d\n", mon->layout); break;
    }
}

