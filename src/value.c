
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "value.h"

heapValue_t *firstHeapValue = NULL;

heapValue_t *createHeapValue(int8_t type) {
    heapValue_t *output = malloc(sizeof(heapValue_t));
    output->type = type;
    output->previous = NULL;
    output->next = firstHeapValue;
    if (firstHeapValue != NULL) {
        firstHeapValue->previous = output;
    }
    output->lockDepth = 0;
    output->referenceCount = 0;
    firstHeapValue = output;
    return output;
}

value_t createValueFromHeapValue(heapValue_t *heapValue) {
    value_t output;
    output.type = heapValue->type;
    output.heapValue = heapValue;
    return output;
}

value_t copyValue(value_t value) {
    // TODO: Implement string and list copy.
    return value;
}

value_t convertCharacterVectorToStringValue(vector_t *vector) {
    heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_STRING);
    tempHeapValue->vector = *vector;
    return createValueFromHeapValue(tempHeapValue);
}

value_t convertTextToStringValue(int8_t *text) {
    int64_t tempLength = strlen((char *)text);
    vector_t tempVector;
    createVectorFromArray(&tempVector, 1, text, tempLength + 1);
    return convertCharacterVectorToStringValue(&tempVector);
}

value_t convertValueToString(value_t value, int8_t shouldCopy) {
    if (value.type == VALUE_TYPE_VOID) {
        return convertTextToStringValue((int8_t *)"(Void)");
    }
    if (value.type == VALUE_TYPE_STRING) {
        if (shouldCopy) {
            return copyValue(value);
        } else {
            return value;
        }
    }
    if (value.type == VALUE_TYPE_NUMBER) {
        double tempNumber = value.numberValue;
        int8_t tempBuffer[50];
        sprintf((char *)tempBuffer, "%lf", tempNumber);
        int32_t tempLength = strlen((char *)tempBuffer);
        int32_t minimumEndIndex = 0;
        while (minimumEndIndex < tempLength) {
            int8_t tempCharacter = tempBuffer[minimumEndIndex];
            if (tempCharacter == '.') {
                break;
            }
            minimumEndIndex += 1;
        }
        int32_t endIndex = tempLength;
        while (endIndex > minimumEndIndex) {
            int8_t tempCharacter = tempBuffer[endIndex - 1];
            if (tempCharacter != '0' && tempCharacter != '.') {
                break;
            }
            endIndex -= 1;
        }
        tempBuffer[endIndex] = 0;
        return convertTextToStringValue(tempBuffer);
    }
    // TODO: Convert more values to strings.
    
    value_t output;
    output.type = VALUE_TYPE_VOID;
    return output;
}

value_t readValueFromAlias(alias_t alias) {
    heapValue_t *heapValue = alias.container;
    int32_t index = (int32_t)(alias.index);
    if (heapValue->type == VALUE_TYPE_FRAME) {
        return heapValue->frameVariableList[index];
    }
    // TODO: Read from more types of heap values.
    
    value_t output;
    output.type = VALUE_TYPE_VOID;
    return output;
}

void writeValueToAlias(alias_t alias, value_t value) {
    heapValue_t *heapValue = alias.container;
    int32_t index = (int32_t)(alias.index);
    if (heapValue->type == VALUE_TYPE_FRAME) {
        heapValue->frameVariableList[index] = value;
    }
    // TODO: Write to more types of heap values.
    
}

value_t resolveAliasValue(value_t value) {
    if (value.type == VALUE_TYPE_ALIAS) {
        return readValueFromAlias(value.alias);
    }
    return value;
}

void writeValueToAliasValue(value_t aliasValue, value_t value) {
    // TODO: Enforce that aliasValue has the correct type.
    writeValueToAlias(aliasValue.alias, value);
}


