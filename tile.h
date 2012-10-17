#ifndef TILE_H
#define TILE_H

#include "global.h"

#define ON_MONITOR(m,c) (m == c->mon)
#define IS_VISIBLE(c)   (BITMASK_CHECK(c->mon->tags, c->tags))
#define IS_TILED(c)     (!c->is_floating && !c->is_fullscrn)

void tile(const monitor_t *mon, const layout_t layout);

#endif
