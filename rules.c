#include <string.h>

#include "rules.h"

rule_t *rule_create(void)
{
    rule_t *rule = calloc(1, sizeof(rule_t));

    if (!rule)
        return (void *)0;

    rule->is_floating = false;
    rule->next = (void *)0;

    return rule;
}

void rules_apply(client_t *c)
{
    for (rule_t *rule = cfg.rules; rule; rule = rule->next)
        if (strstr(rule->class_instance, c->class) || strstr(rule->class_instance, c->instance))
            c->is_floating = rule->is_floating;
}

