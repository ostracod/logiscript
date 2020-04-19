
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "function.h"
#include "variable.h"

void scopeAddVariable(scope_t *scope, int8_t *name) {
    scopeVariable_t *tempVariable = malloc(sizeof(scopeVariable_t));
    tempVariable->name = name;
    tempVariable->scopeIndex = scope->variableList.length;
    tempVariable->parentScopeIndex = -1;
    pushVectorElement(&(scope->variableList), &tempVariable);
}

scopeVariable_t *scopeFindVariable(scope_t *scope, int8_t *name) {
    for (int32_t index = 0; index < scope->variableList.length; index++) {
        scopeVariable_t *tempVariable;
        getVectorElement(&tempVariable, &(scope->variableList), index);
        if (strcmp((char *)name, (char *)(tempVariable->name)) == 0) {
            return tempVariable;
        }
    }
    return NULL;
}


