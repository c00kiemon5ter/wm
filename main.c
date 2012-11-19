#include "main.h"

int main(int argc, char *argv[]) {
    if (argc == 1)
        return cookiewm();
    else
        return cookie(argc, argv);
}

