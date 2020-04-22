
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "error.h"

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


