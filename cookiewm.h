#ifndef _COOKIEWM_H
#define _COOKIEWM_H

#include <stdbool.h>
#include <xcb/xcb.h>

static bool running;
static int nmonitors;

static xcb_screen_t *screen;
static xcb_connection_t *dpy;
static void register_events(void);

static void setup(void);
static void register_events(void);
static void quit(void);

#endif
