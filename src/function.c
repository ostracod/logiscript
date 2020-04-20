
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "function.h"
#include "evaluate.h"

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

heapValue_t *createFunctionHandle(customFunction_t *customFunction) {
    customFunctionHandle_t *tempHandle = malloc(sizeof(customFunctionHandle_t));
    tempHandle->function = customFunction;
    tempHandle->aliasList = NULL; // TODO: Populate this.
    heapValue_t *output = createHeapValue(VALUE_TYPE_CUSTOM_FUNCTION);
    output->customFunctionHandle = tempHandle;
    return output;
}

heapValue_t *invokeFunctionHandle(
    heapValue_t *functionHandle,
    value_t *argumentList,
    int32_t argumentCount
) {
    customFunctionHandle_t *tempHandle = functionHandle->customFunctionHandle;
    customFunction_t *customFunction = tempHandle->function;
    // TODO: Populate alias variables in frame.
    heapValue_t *tempFrame = scopeCreateFrame(&(customFunction->scope));
    for (int32_t index = 0; index < argumentCount; index++) {
        tempFrame->frameVariableList[index] = argumentList[index];
    }
    for (int64_t index = 0; index < customFunction->statementList.length; index++) {
        baseStatement_t *tempStatement;
        getVectorElement(&tempStatement, &(customFunction->statementList), index);
        evaluateStatement(tempFrame, tempStatement);
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


