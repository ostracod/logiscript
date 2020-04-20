
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

heapValue_t *scopeCreateFrame(scope_t *scope) {
    int32_t tempLength = (int32_t)(scope->variableList.length);
    frameVariable_t *frameVariableList = malloc(sizeof(frameVariable_t) * tempLength);
    for (int32_t index = 0; index < tempLength; index++) {
        frameVariable_t tempVariable;
        tempVariable.type = FRAME_VARIABLE_TYPE_VALUE;
        tempVariable.value.type = VALUE_TYPE_VOID;
        frameVariableList[index] = tempVariable;
    }
    // TODO: Populate alias variables.
    
    heapValue_t *output = createHeapValue(VALUE_TYPE_FRAME);
    output->frameVariableList = frameVariableList;
    return output;
}

aliasedValue_t readFrameVariable(heapValue_t *frame, int32_t index) {
    frameVariable_t *frameVariable = frame->frameVariableList + index;
    aliasedValue_t output;
    // TODO: Handle alias variables.
    output.value = frameVariable->value;
    output.alias.container = frame;
    output.alias.index = index;
    return output;
}

void writeFrameVariable(heapValue_t *frame, int32_t index, value_t value) {
    frameVariable_t *frameVariable = frame->frameVariableList + index;
    // TODO: Handle alias variables.
    frameVariable->value = value;
}


