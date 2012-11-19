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
void grid(const monitor_t *mon, const unsigned short wins)
{
    const uint16_t fix = !!mon->spacer * OFFSET;  /* single border/spacer in the middle */
    unsigned short cols = 0, rows = 0;
    unsigned short ncol = 0, nrow = 0;
    unsigned short i = 0;

    /* calculate number of columns and rows - emulate square root */
    for (cols = 0; cols <= wins/2; cols++)
        if (cols*cols >= wins)
            break;

    if (wins == 5)
        cols = 2;
    rows = wins/cols;

    xcb_rectangle_t r = mon->geom;

    r.x      = mon->geom.x + mon->spacer;
    r.y      = mon->geom.y + mon->spacer;
    r.width  = ((mon->geom.width - (mon->spacer ? mon->spacer : mon->border)) / (cols + !cols)) - mon->border - fix;
    r.height = mon->geom.height  - (mon->spacer ? mon->spacer : mon->border);

    for (client_t *c = cfg.vlist; c && i < wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            if (i++/rows + 1 > cols - wins%cols)
                rows = wins/cols + 1;

            client_move_resize(c, r.x + ncol * (r.width + mon->border + fix),
                                  r.y + nrow * r.height / rows, r.width,
                                  r.height / rows - mon->border - fix);

            /* all rows filled - reset row count and go to next column */
            if (++nrow >= rows) {
                nrow = 0;
                ncol++;
            }
        }
}

/**
 * vertical stack - the common tiling layout
 * supports multiple windows in master area
 * aka nv-stack nmaster-vertical/tile-stack
 *
 * +--------+
 * |    |   |
 * |    +---|
 * |    |   |
 * +--------+
 *
 */
void vstack(const monitor_t *mon, const unsigned short wins)
{
    const uint16_t m_area = mon->geom.width * M_AREA_FACT + mon->m_area;
    const uint16_t fix = !!mon->spacer * OFFSET;  /* single border/spacer in the middle */

    uint16_t m_wins = (wins <= mon->m_wins) ? wins - 1 : mon->m_wins;
    uint16_t s_wins = wins - m_wins;

    client_t *c = cfg.vlist;
    xcb_rectangle_t r;

    r.x      = mon->geom.x + mon->spacer;
    r.y      = mon->geom.y + mon->spacer;
    r.width  = m_area - 2 * OFFSET;
    r.height = ((mon->geom.height - (mon->spacer ? mon->spacer : mon->border)) / m_wins) - mon->border - fix;

    /* place master 'm_wins' windows in the master area */
    for (; c && m_wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_move_resize_geom(c, r);
            r.y += mon->border + r.height + fix;
            --m_wins;
        }

    r.x      = mon->geom.x + OFFSET + r.width + fix;
    r.y      = mon->geom.y + mon->spacer;
    r.width  = mon->geom.width - m_area - mon->border - fix;
    r.height = ((mon->geom.height - (mon->spacer ? mon->spacer : mon->border)) / s_wins) - mon->border - fix;

    /* place stack 's_wins' windows in the stack area */
    for (; c && s_wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_move_resize_geom(c, r);
            r.y += mon->border + r.height + fix;
            --s_wins;
        }
}

/**
 * horizontal stack - bottom stack
 * supports multiple windows in master area
 * aka nh-stack nmaster-bottom-stack
 *
 * +--------+
 * |        |
 * |--+--+--|
 * |  |  |  |
 * +--------+
 *
 */
void hstack(const monitor_t *mon, const unsigned short wins)
{
    const uint16_t m_area = mon->geom.height * M_AREA_FACT + mon->m_area;
    const uint16_t fix = !!mon->spacer * OFFSET;  /* single border/spacer in the middle */

    uint16_t m_wins = (wins <= mon->m_wins) ? wins - 1 : mon->m_wins;
    uint16_t s_wins = wins - m_wins;

    client_t *c = cfg.vlist;
    xcb_rectangle_t r;

    r.x      = mon->geom.x + mon->spacer;
    r.y      = mon->geom.y + mon->spacer;
    r.width  = ((mon->geom.width - (mon->spacer ? mon->spacer : mon->border)) / m_wins) - mon->border - fix;
    r.height = m_area - 2 * OFFSET;

    /* place master 'm_wins' windows in the master area */
    for (; c && m_wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_move_resize_geom(c, r);
            r.x += mon->border + r.width + fix;
            --m_wins;
        }

    r.x      = mon->geom.x + mon->spacer;
    r.y      = mon->geom.y + OFFSET + r.height + fix;
    r.width  = ((mon->geom.width - (mon->spacer ? mon->spacer : mon->border)) / s_wins) - mon->border - fix;
    r.height = mon->geom.height - m_area - mon->border - fix;

    /* place stack 's_wins' windows in the stack area */
    for (; c && s_wins; c = c->vnext)
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c)) {
            client_move_resize_geom(c, r);
            r.x += mon->border + r.width + fix;
            --s_wins;
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
        if (ON_MONITOR(mon, c) && IS_VISIBLE(c) && IS_TILED(c))
                ++num_windows;

    PRINTF("found '%u' windows to tile on monitor\n", num_windows);

    /* if no windows there is nothing to arrange */
    if (num_windows == 0)
        return;

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

