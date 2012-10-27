#include <stdlib.h>
#include <string.h>

#include "helpers.h"

char *bitstr(uintmax_t val, char bits[static BUF_BITS_SIZE])
{
    static const short base = 2;
    char *str = bits + BUF_BITS_SIZE;

    PRINTF("dec: %zd\n", val);
    PRINTF("hex: %jx\n", val);

    for (*--str = '\0'; val; val /= base)
        *--str = '0' + val % base;
    if (!*str)
        *--str = '0';
    for (char *i = bits; i != str; i++)
        *i = '0';

    PRINTF("bin: %s\n", str);
    PRINTF("bit: %s\n", bits);

    return str;
}

inline
uintmax_t max(uintmax_t a, uintmax_t b)
{
    return (a > b) ? a : b;
}

inline
uintmax_t min(uintmax_t a, uintmax_t b)
{
    return (a < b) ? a : b;
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

