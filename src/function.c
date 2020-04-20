
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "function.h"
#include "evaluate.h"
#include "error.h"

builtInFunction_t builtInFunctionSet[] = {
    {{FUNCTION_TYPE_BUILT_IN, 2}, (int8_t *)"SET", BUILT_IN_FUNCTION_SET},
    {{FUNCTION_TYPE_BUILT_IN, 2}, (int8_t *)"TYPE", BUILT_IN_FUNCTION_TYPE},
    {{FUNCTION_TYPE_BUILT_IN, 3}, (int8_t *)"CONVERT", BUILT_IN_FUNCTION_CONVERT},
    {{FUNCTION_TYPE_BUILT_IN, 2}, (int8_t *)"SIZE", BUILT_IN_FUNCTION_SIZE},
    {{FUNCTION_TYPE_BUILT_IN, 2}, (int8_t *)"RESIZE", BUILT_IN_FUNCTION_RESIZE},
    {{FUNCTION_TYPE_BUILT_IN, 2}, (int8_t *)"IF", BUILT_IN_FUNCTION_IF},
    {{FUNCTION_TYPE_BUILT_IN, 1}, (int8_t *)"LOOP", BUILT_IN_FUNCTION_LOOP},
    {{FUNCTION_TYPE_BUILT_IN, 2}, (int8_t *)"THROW", BUILT_IN_FUNCTION_THROW},
    {{FUNCTION_TYPE_BUILT_IN, 3}, (int8_t *)"CATCH", BUILT_IN_FUNCTION_CATCH},
    {{FUNCTION_TYPE_BUILT_IN, 1}, (int8_t *)"PRINT", BUILT_IN_FUNCTION_PRINT},
    {{FUNCTION_TYPE_BUILT_IN, 1}, (int8_t *)"PROMPT", BUILT_IN_FUNCTION_PROMPT}
};

builtInFunction_t *findBuiltInFunctionByName(int8_t *name) {
    int32_t tempLength = sizeof(builtInFunctionSet) / sizeof(*builtInFunctionSet);
    for (int32_t index = 0; index < tempLength; index++) {
        builtInFunction_t *tempFunction = builtInFunctionSet + index;
        if (strcmp((char *)name, (char *)tempFunction->name) == 0) {
            return tempFunction;
        }
    }
    return NULL;
}

void invokeBuiltInFunction(
    builtInFunction_t *builtInFunction,
    value_t *argumentList,
    int32_t argumentCount
) {
    // TODO: Validate argumentCount.
    
    switch (builtInFunction->number) {
        case BUILT_IN_FUNCTION_SET:
        {
            value_t tempValue = resolveAliasValue(argumentList[1]);
            writeValueToAliasValue(argumentList[0], tempValue);
            break;
        }
        case BUILT_IN_FUNCTION_IF:
        {
            value_t tempCondition = resolveAliasValue(argumentList[0]);
            if (tempCondition.numberValue != 0) {
                value_t tempHandle = resolveAliasValue(argumentList[1]);
                invokeFunctionHandle(tempHandle.heapValue, NULL, 0);
            }
            break;
        }
        case BUILT_IN_FUNCTION_LOOP:
        {
            value_t tempHandle = resolveAliasValue(argumentList[0]);
            while (!hasThrownError) {
                invokeFunctionHandle(tempHandle.heapValue, NULL, 0);
            }
            break;
        }
        case BUILT_IN_FUNCTION_THROW:
        {
            value_t tempChannel = resolveAliasValue(argumentList[0]);
            value_t tempValue = resolveAliasValue(argumentList[1]);
            throwError((int32_t)(tempChannel.numberValue), tempValue);
            break;
        }
        case BUILT_IN_FUNCTION_CATCH:
        {
            value_t tempChannel = resolveAliasValue(argumentList[1]);
            value_t tempHandle = resolveAliasValue(argumentList[2]);
            invokeFunctionHandle(tempHandle.heapValue, NULL, 0);
            if (hasThrownError && (int32_t)(tempChannel.numberValue) == thrownErrorChannel) {
                writeValueToAliasValue(argumentList[0], thrownErrorValue);
                hasThrownError = false;
            }
            break;
        }
        case BUILT_IN_FUNCTION_PRINT:
        {
            value_t tempValue = resolveAliasValue(argumentList[0]);
            value_t stringValue = convertValueToString(tempValue, false);
            printf("%s\n", stringValue.heapValue->vector.data);
            break;
        }
        // TODO: Implement more built-in functions.
        
        default:
        {
            break;
        }
    }
}

heapValue_t *createFunctionHandle(heapValue_t *frame, customFunction_t *customFunction) {
    customFunctionHandle_t *tempHandle = malloc(sizeof(customFunctionHandle_t));
    tempHandle->function = customFunction;
    scope_t *tempScope = &(customFunction->scope);
    int32_t tempLength = tempScope->aliasVariableAmount;
    tempHandle->aliasList = malloc(sizeof(alias_t) * tempLength);
    int32_t scopeIndex = 0;
    int32_t aliasIndex = 0;
    while (scopeIndex < tempScope->variableList.length && aliasIndex < tempLength) {
        scopeVariable_t *tempVariable;
        getVectorElement(&tempVariable, &(tempScope->variableList), scopeIndex);
        scopeIndex += 1;
        if (tempVariable->parentScopeIndex < 0) {
            continue;
        }
        tempHandle->aliasList[aliasIndex] = getAliasToFrameVariable(
            frame,
            tempVariable->parentScopeIndex
        );
        aliasIndex += 1;
    }
    heapValue_t *output = createHeapValue(VALUE_TYPE_CUSTOM_FUNCTION);
    output->customFunctionHandle = tempHandle;
    return output;
}

heapValue_t *functionHandleCreateFrame(customFunctionHandle_t *functionHandle) {
    customFunction_t *customFunction = functionHandle->function;
    alias_t *aliasList = functionHandle->aliasList;
    scope_t *tempScope = &(customFunction->scope);
    int32_t tempLength = (int32_t)(tempScope->variableList.length);
    value_t *frameVariableList = malloc(sizeof(value_t) * tempLength);
    int32_t scopeIndex = 0;
    int32_t aliasIndex = 0;
    while (scopeIndex < tempLength) {
        scopeVariable_t *scopeVariable;
        getVectorElement(&scopeVariable, &(tempScope->variableList), scopeIndex);
        value_t tempValue;
        if (scopeVariable->parentScopeIndex >= 0) {
            tempValue.type = VALUE_TYPE_ALIAS;
            tempValue.alias = aliasList[aliasIndex];
            aliasIndex += 1;
        } else {
            tempValue.type = VALUE_TYPE_VOID;
        }
        frameVariableList[scopeIndex] = tempValue;
        scopeIndex += 1;
    }
    heapValue_t *output = createHeapValue(VALUE_TYPE_FRAME);
    output->frameVariableList = frameVariableList;
    return output;
}

heapValue_t *invokeFunctionHandle(
    heapValue_t *functionHandle,
    value_t *argumentList,
    int32_t argumentCount
) {
    customFunctionHandle_t *tempHandle = functionHandle->customFunctionHandle;
    heapValue_t *tempFrame = functionHandleCreateFrame(tempHandle);
    for (int32_t index = 0; index < argumentCount; index++) {
        tempFrame->frameVariableList[index] = argumentList[index];
    }
    customFunction_t *customFunction = tempHandle->function;
    for (int64_t index = 0; index < customFunction->statementList.length; index++) {
        baseStatement_t *tempStatement;
        getVectorElement(&tempStatement, &(customFunction->statementList), index);
        evaluateStatement(tempFrame, tempStatement);
        if (hasThrownError) {
            break;
        }
    }
    return tempFrame;
}

void invokeFunction(
    value_t functionValue,
    value_t *argumentList,
    int32_t argumentCount
) {
    if (functionValue.type == VALUE_TYPE_BUILT_IN_FUNCTION) {
        invokeBuiltInFunction(
            functionValue.builtInFunction,
            argumentList,
            argumentCount
        );
        return;
    }
    if (functionValue.type == VALUE_TYPE_CUSTOM_FUNCTION) {
        invokeFunctionHandle(functionValue.heapValue, argumentList, argumentCount);
        return;
    }
    // TODO: Throw an error if the given value is not a function.
    
}


