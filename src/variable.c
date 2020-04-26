
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "function.h"
#include "variable.h"
#include "error.h"

scopeVariable_t *scopeAddVariable(
    scope_t *scope,
    int8_t *name,
    int32_t parentScopeIndex,
    int8_t allowDuplicates
) {
    scopeVariable_t *tempVariable = scopeFindVariable(scope, name);
    if (tempVariable != NULL) {
        if (allowDuplicates) {
            return tempVariable;
        } else {
            THROW_BUILT_IN_ERROR(
                DATA_ERROR_CONSTANT,
                "Duplicate variable \"%s\".",
                name
            );
            return NULL;
        }
    }
    scopeVariable_t *output = malloc(sizeof(scopeVariable_t));
    output->name = name;
    output->scopeIndex = scope->variableList.length;
    output->parentScopeIndex = parentScopeIndex;
    pushVectorElement(&(scope->variableList), &output);
    if (parentScopeIndex >= 0) {
        scope->aliasVariableAmount += 1;
    }
    return output;
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

alias_t getAliasToFrameVariable(heapValue_t *frame, int32_t index) {
    hyperValue_t tempValue = frame->frameVariableList.valueArray[index];
    if (tempValue.type == HYPER_VALUE_TYPE_ALIAS) {
        return tempValue.alias;
    } else {
        alias_t tempAlias;
        tempAlias.container = frame;
        tempAlias.index = index;
        return tempAlias;
    }
}


