#include <stdio.h>

#include "messages.h"

void process_message(char *msg, char *rsp)
{
    printf(" --- Got msg: %s\nNo responce: %s\n", msg, rsp);
}

