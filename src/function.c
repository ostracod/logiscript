
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

hyperValueList_t emptyArgumentList = {NULL, 0};

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

void checkInvocationArgumentList(baseFunction_t *function, hyperValueList_t **argumentList) {
    if (*argumentList == NULL) {
        *argumentList = &emptyArgumentList;
    }
    int32_t argumentAmount = function->argumentAmount;
    if ((*argumentList)->length > argumentAmount) {
        if (argumentAmount == 1) {
            THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Expected at most 1 argument.");
        } else {
            THROW_BUILT_IN_ERROR(
                DATA_ERROR_CONSTANT,
                "Expected at most %d arguments.",
                argumentAmount
            );
        }
    }
}

hyperValue_t getArgument(hyperValueList_t *argumentList, int32_t index) {
    if (index < argumentList->length) {
        return argumentList->valueArray[index];
    }
    hyperValue_t tempValue;
    tempValue.type = HYPER_VALUE_TYPE_VALUE;
    tempValue.value.type = VALUE_TYPE_VOID;
    return tempValue;
}

// Note: This will throw an error if the hyper value
// contains an alias with a bad index.
value_t getResolvedArgument(hyperValueList_t *argumentList, int32_t index) {
    hyperValue_t tempValue = getArgument(argumentList, index);
    return readValueFromHyperValue(tempValue);
}

void invokeBuiltInFunction(
    builtInFunction_t *builtInFunction,
    hyperValueList_t *argumentList
) {
    checkInvocationArgumentList(&(builtInFunction->base), &argumentList);
    if (hasThrownError) {
        return;
    }
    switch (builtInFunction->number) {
        case BUILT_IN_FUNCTION_SET:
        {
            value_t tempValue = getResolvedArgument(argumentList, 1);
            if (hasThrownError) {
                return;
            }
            writeValueToLocation(getArgument(argumentList, 0), tempValue);
            break;
        }
        case BUILT_IN_FUNCTION_IF:
        {
            value_t tempCondition = getResolvedArgument(argumentList, 0);
            value_t tempHandle = getResolvedArgument(argumentList, 1);
            if (hasThrownError) {
                return;
            }
            if (tempCondition.type != VALUE_TYPE_NUMBER) {
                THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Condition must be a number.");
                return;
            }
            if (tempCondition.numberValue != 0) {
                invokeFunction(tempHandle, NULL);
            }
            break;
        }
        case BUILT_IN_FUNCTION_LOOP:
        {
            value_t tempHandle = getResolvedArgument(argumentList, 0);
            while (!hasThrownError) {
                invokeFunction(tempHandle, NULL);
            }
            break;
        }
        case BUILT_IN_FUNCTION_THROW:
        {
            value_t tempChannel = getResolvedArgument(argumentList, 0);
            value_t tempValue = getResolvedArgument(argumentList, 1);
            if (hasThrownError) {
                return;
            }
            if (tempChannel.type != VALUE_TYPE_NUMBER) {
                THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Channel must be a number.");
                return;
            }
            throwError((int32_t)(tempChannel.numberValue), tempValue);
            break;
        }
        case BUILT_IN_FUNCTION_CATCH:
        {
            hyperValue_t tempDestination = getArgument(argumentList, 0);
            value_t tempChannel = getResolvedArgument(argumentList, 1);
            value_t tempHandle = getResolvedArgument(argumentList, 2);
            if (hasThrownError) {
                return;
            }
            if (tempChannel.type != VALUE_TYPE_NUMBER) {
                THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Channel must be a number.");
                return;
            }
            invokeFunction(tempHandle, NULL);
            if (hasThrownError) {
                if ((int32_t)(tempChannel.numberValue) == thrownErrorChannel) {
                    hasThrownError = false;
                    writeValueToLocation(tempDestination, thrownErrorValue);
                    unlockValue(&thrownErrorValue);
                }
            } else {
                value_t tempValue;
                tempValue.type = VALUE_TYPE_VOID;
                writeValueToLocation(tempDestination, tempValue);
            }
            break;
        }
        case BUILT_IN_FUNCTION_PRINT:
        {
            value_t tempValue = getResolvedArgument(argumentList, 0);
            if (hasThrownError) {
                return;
            }
            value_t stringValue = convertValueToString(tempValue, false);
            printf("%s\n", stringValue.heapValue->vector.data);
            deleteValueIfUnreferenced(&stringValue);
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
    int32_t tempLength = tempScope->parentVariableAmount;
    hyperValue_t *tempLocationArray = malloc(sizeof(hyperValue_t) * tempLength);
    tempHandle->locationList.valueArray = tempLocationArray;
    tempHandle->locationList.length = tempLength;
    int32_t scopeIndex = 0;
    int32_t locationIndex = 0;
    while (scopeIndex < tempScope->variableList.length && locationIndex < tempLength) {
        scopeVariable_t *tempVariable;
        getVectorElement(&tempVariable, &(tempScope->variableList), scopeIndex);
        scopeIndex += 1;
        if (tempVariable->parentScopeIndex < 0) {
            continue;
        }
        hyperValue_t tempLocation = getFrameVariableLocation(
            frame,
            tempVariable->parentScopeIndex
        );
        tempLocationArray[locationIndex] = tempLocation;
        addHyperValueReference(&tempLocation);
        locationIndex += 1;
    }
    heapValue_t *output = createHeapValue(VALUE_TYPE_CUSTOM_FUNCTION);
    output->customFunctionHandle = tempHandle;
    return output;
}

heapValue_t *functionHandleCreateFrame(customFunctionHandle_t *functionHandle) {
    scope_t *tempScope = &(functionHandle->function->scope);
    hyperValue_t *tempLocationArray = functionHandle->locationList.valueArray;
    int32_t tempLength = (int32_t)(tempScope->variableList.length);
    hyperValue_t *tempValueArray = malloc(sizeof(hyperValue_t) * tempLength);
    for (int32_t index = 0; index < tempLength; index++) {
        hyperValue_t tempValue;
        tempValue.type = HYPER_VALUE_TYPE_VALUE;
        tempValue.value.type = VALUE_TYPE_VOID;
        tempValueArray[index] = tempValue;
    }
    int32_t scopeIndex = 0;
    int32_t locationIndex = 0;
    while (scopeIndex < tempLength) {
        scopeVariable_t *scopeVariable;
        getVectorElement(&scopeVariable, &(tempScope->variableList), scopeIndex);
        if (scopeVariable->parentScopeIndex >= 0) {
            hyperValue_t *tempLocation = tempLocationArray + locationIndex;
            swapHyperValueReference(tempValueArray + scopeIndex, tempLocation);
            locationIndex += 1;
        }
        scopeIndex += 1;
    }
    heapValue_t *output = createHeapValue(VALUE_TYPE_FRAME);
    output->frameVariableList.valueArray = tempValueArray;
    output->frameVariableList.length = tempLength;
    return output;
}

// Output will be locked.
heapValue_t *invokeFunctionHandle(
    heapValue_t *functionHandle,
    hyperValueList_t *argumentList
) {
    customFunctionHandle_t *tempHandle = functionHandle->customFunctionHandle;
    customFunction_t *customFunction = tempHandle->function;
    checkInvocationArgumentList(&(customFunction->base), &argumentList);
    if (hasThrownError) {
        return NULL;
    }
    heapValue_t *tempFrame = functionHandleCreateFrame(tempHandle);
    lockHeapValue(tempFrame);
    for (int32_t index = 0; index < argumentList->length; index++) {
        swapHyperValueReference(
            tempFrame->frameVariableList.valueArray + index,
            argumentList->valueArray + index
        );
    }
    for (int64_t index = 0; index < customFunction->statementList.length; index++) {
        baseStatement_t *tempStatement;
        getVectorElement(&tempStatement, &(customFunction->statementList), index);
        evaluateStatement(tempFrame, tempStatement);
        if (hasThrownError) {
            addBodyPosToStackTrace(&(tempStatement->bodyPos));
            break;
        }
    }
    return tempFrame;
}

void invokeFunction(value_t functionValue, hyperValueList_t *argumentList) {
    if (functionValue.type == VALUE_TYPE_BUILT_IN_FUNCTION) {
        invokeBuiltInFunction(functionValue.builtInFunction, argumentList);
        return;
    }
    if (functionValue.type == VALUE_TYPE_CUSTOM_FUNCTION) {
        heapValue_t *tempFrame = invokeFunctionHandle(functionValue.heapValue, argumentList);
        unlockHeapValue(tempFrame);
        return;
    }
    THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected function handle.");
}


