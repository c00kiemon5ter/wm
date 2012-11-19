#ifndef RULES_H
#define RULES_H

#include <stdbool.h>

#include "wm.h"

rule_t *rule_create(void);
void rules_apply(client_t *);

#endif
