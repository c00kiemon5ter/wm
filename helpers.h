#ifndef HELPERS_H
#define HELPERS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#define XCB_CONFIG_WINDOW_X_Y_WIDTH_HEIGHT XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT
#define XCB_CONFIG_WINDOW_X_Y XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y

#define LENGTH(x)       (sizeof(x) / sizeof(*x))
#define MAX(A, B)       ((A) > (B) ? (A) : (B))
#define MIN(A, B)       ((A) < (B) ? (A) : (B))
#define BOOLSTR(A)      ((A) ? "true" : "false")

#ifdef DEBUG
#  define PRINTF(fmt,...) printf(":: %5d:%15.15s: " fmt, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
#  define PRINTF(fmt,...) ((void)0)
#endif

void warn(char *, ...);
__attribute__((noreturn))
void err(char *, ...);
uint32_t get_color(char *);

#endif
