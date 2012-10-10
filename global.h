#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

/* TODO global list
 * revisit BUFLEN ? BUFSIZ ?
 *
 * Never use a signed value to index an array or to store its bounds
 * Use size_t to index an array or to store its bounds
 *
 * Use [u]intX_t whenever you make an assumption about the width of a type (ie bitfield)
 *
 * When you need a `small’ integer type don’t use char but use [u]int8_t.
 *
 * Use the correct types to transform a pointer to an integer.
 * These have predefined names, too, intptr_t and uintptr_t, if they exist.
 * When you do pointer arithmetic, use ptrdiff_t for differences between pointers
 *
 * Use 0 inplace of NULL
 *
 * Use the different incarnations of 0 (0, '\0', false, 0.0, null pointer) as a default value of variables and fields.
 * Use a scope that is as narrow as possible to declare variables.
 * Use catch-all inintializer.  Foo *foo = { bar }
 * Use designated initializers. Foo *foo = { .foo = bar, [15] = 0x02 }
 * Use initializers instead of assignments wherever this is possible.
 * Use constant compound literals to assign to a structure as a whole instead of assigning to each field. Foo *foo; foo = (Foo){ bar }
 *
 * union overlay {
 *     T x;
 *     unsigned char X[sizeof(T)];
 * };
 * will always allow you to access each individual bit of x, regardless what type T represents
 *
 */

#define BUFLEN 256

typedef struct {
    int x, y, w, h;
} rectangle_t;

typedef struct client_t {
    rectangle_t geom;
    xcb_window_t win;
    struct client_t *next;
} client_t;

typedef struct monitor_t {
    rectangle_t geom;
    unsigned int tags;
    struct monitor_t *next;
} monitor_t;

typedef struct {
    bool running;

    char wm_name[BUFLEN];
    int default_screen;

    xcb_screen_t *screen;
    xcb_connection_t *connection;

    xcb_ewmh_connection_t *ewmh;
} global_configuration_t;

extern global_configuration_t cfg;

#endif
