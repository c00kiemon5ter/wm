#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

#define XCB_CONFIG_WINDOW_MOVE          XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#define XCB_CONFIG_WINDOW_RESIZE        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define XCB_CONFIG_WINDOW_MOVE_RESIZE   XCB_CONFIG_WINDOW_MOVE | XCB_CONFIG_WINDOW_RESIZE

#define LENGTH(x)           (sizeof(x) / sizeof(*x))
#define BOOLSTR(x)          &("false\0true"[6*x])

#ifdef DEBUG
#define PRINTF(fmt,...)     printf(":: %5d:%15.15s: " fmt, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
#define PRINTF(fmt,...)     ((void)0)
#endif

#define BIT_SET(x,b)        ((x) |= (1 << (b)))
#define BIT_FLIP(x,b)       ((x) ^= (1 << (b)))
#define BIT_CLEAR(x,b)      ((x) &= (1 << (b)))
#define BIT_CHECK(x,b)      ((x) &  (1 << (b)))

#define BITMASK_SET(x,m)    ((x) |=   (m))
#define BITMASK_FLIP(x,m)   ((x) ^=   (m))
#define BITMASK_CLEAR(x,m)  ((x) &= (~(m)))
#define BITMASK_CHECK(x,m)  ((x) &    (m))

#define BUF_BITS_SIZE        (CHAR_BIT * sizeof(uintmax_t) + 1)
char *bitstr(uintmax_t, char [static BUF_BITS_SIZE]);

uintmax_t max(uintmax_t, uintmax_t);
uintmax_t min(uintmax_t, uintmax_t);

void warn(char *, ...);
__attribute__((noreturn)) void err(char *, ...);

uint32_t get_color(char *);

#endif
