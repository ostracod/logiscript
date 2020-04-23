
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "error.h"
#include "resolve.h"

errorConstant_t errorConstantSet[] = {
    {(int8_t *)"TYPE_ERROR", TYPE_ERROR_CONSTANT},
    {(int8_t *)"NUMBER_ERROR", NUMBER_ERROR_CONSTANT},
    {(int8_t *)"DATA_ERROR", DATA_ERROR_CONSTANT},
    {(int8_t *)"MISSING_ERROR", MISSING_ERROR_CONSTANT}
};

errorConstant_t *findErrorConstantByCode(int32_t errorCode) {
    int32_t tempLength = sizeof(errorConstantSet) / sizeof(*errorConstantSet);
    for (int32_t index = 0; index < tempLength; index++) {
        errorConstant_t *tempConstant = errorConstantSet + index;
        if (tempConstant->code == errorCode) {
            return tempConstant;
        }
    }
    return NULL;
}

void addErrorConstantsToNumberConstants(vector_t *destination) {
    int32_t tempLength = sizeof(errorConstantSet) / sizeof(*errorConstantSet);
    for (int32_t index = 0; index < tempLength; index++) {
        errorConstant_t *tempErrorConstant = errorConstantSet + index;
        numberConstant_t tempNumberConstant;
        tempNumberConstant.name = tempErrorConstant->name;
        tempNumberConstant.value = tempErrorConstant->code;
        pushVectorElement(destination, &tempNumberConstant);
    }
}

void throwError(int32_t channel, value_t value) {
    thrownErrorChannel = channel;
    thrownErrorValue = value;
    hasThrownError = true;
}

void throwBuiltInError(int32_t errorCode, int8_t *message) {
    value_t tempNumberValue;
    tempNumberValue.type = VALUE_TYPE_NUMBER;
    tempNumberValue.numberValue = errorCode;
    value_t tempStringValue = convertTextToStringValue(message);
    vector_t tempVector;
    createEmptyVector(&tempVector, sizeof(value_t));
    pushVectorElement(&tempVector, &tempNumberValue);
    pushVectorElement(&tempVector, &tempStringValue);
    heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_LIST);
    tempHeapValue->vector = tempVector;
    value_t tempList = createValueFromHeapValue(tempHeapValue);
    throwError(ERROR_CHANNEL_CONSTANT, tempList);
}

void printBuiltInError(int32_t errorCode, int8_t *message) {
    errorConstant_t *tempErrorConstant = findErrorConstantByCode(errorCode);
    if (tempErrorConstant == NULL) {
        printf("Uncaught error (code = %d): %s\n", errorCode, message);
    } else {
        printf("Uncaught %s: %s\n", tempErrorConstant->name, message);
    }
}

void printStackTrace() {
    int8_t hasPrintedError = false;
    if (thrownErrorChannel == 0 && thrownErrorValue.type == VALUE_TYPE_LIST) {
        vector_t *tempVector = &(thrownErrorValue.heapValue->vector);
        if (tempVector->length == 2) {
            value_t tempNumberValue;
            value_t tempStringValue;
            getVectorElement(&tempNumberValue, tempVector, 0);
            getVectorElement(&tempStringValue, tempVector, 1);
            if (tempNumberValue.type == VALUE_TYPE_NUMBER
                    && tempStringValue.type == VALUE_TYPE_STRING) {
                printBuiltInError(
                    tempNumberValue.numberValue,
                    tempStringValue.heapValue->vector.data
                );
                hasPrintedError = true;
            }
        }
    }
    if (!hasPrintedError) {
        value_t tempStringValue = convertValueToString(thrownErrorValue, false);
        int8_t *tempText = tempStringValue.heapValue->vector.data;
        printf("Uncaught value (channel = %d): %s\n", thrownErrorChannel, tempText);
    }
    // TODO: Print the rest of the stack trace.
    
}


