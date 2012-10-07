#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

#include "helpers.h"

void warn(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

__attribute__((noreturn))
void err(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

