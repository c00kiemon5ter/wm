#include <string.h>

#include "messages.h"
#include "cookiewm.h"
#include "helpers.h"

void process_message(char *msg, char *rsp)
{
    PRINTF("got message: %s\n", msg);
    if (strcmp(msg, "quit") == 0)
        quit();
    PRINTF("send response: %s\n", rsp);
}

