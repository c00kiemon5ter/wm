#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

int usage(void) {
    puts("man " WM_NAME " # RTFM!");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    if (argc == 1)
        return cookiewm();
    else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
        return usage();
    else
        return cookie(argc, argv);
}

