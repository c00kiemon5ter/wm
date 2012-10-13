#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#define XCB_CONFIG_WINDOW_X_Y_WIDTH_HEIGHT XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define XCB_CONFIG_WINDOW_X_Y XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y

#define LENGTH(x)           (sizeof(x) / sizeof(*x))
#define BOOLSTR(x)          ("false\0true" + 6 * x)

#define MAX(A, B)           ((A) > (B) ? (A) : (B))
#define MIN(A, B)           ((A) < (B) ? (A) : (B))

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

void bitchars(int value);
void warn(char *, ...);
__attribute__((noreturn))
void err(char *, ...);
uint32_t get_color(char *);

#endif
