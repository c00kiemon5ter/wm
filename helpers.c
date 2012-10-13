#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

#include "helpers.h"

void bitchars(int value)
{
    int size = (sizeof(value) * CHAR_BIT);
    char bits[size + 1];
    bits[size] = '\0';

    for (int i = 0; i < size; i++)
        bits[size - 1 - i] = (!!BIT_CHECK(value, i)) + '0';

    PRINTF("dec: %5d :: hex: %#05x :: bin: %s\n", value, value, bits);
}

void warn(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void err(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

