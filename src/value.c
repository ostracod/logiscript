
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "value.h"
#include "error.h"

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
    output->aliasCount = 0;
    firstHeapValue = output;
    return output;
}

void removeValueReferenceHelper(value_t *value, int8_t shouldRecur);

void deleteHeapValue(heapValue_t *value, int8_t shouldRecur) {
    if (value->previous == NULL) {
        firstHeapValue = value->next;
    } else {
        value->previous->next = value->next;
    }
    if (value->next != NULL) {
        value->next->previous = value->previous;
    }
    switch (value->type) {
        case VALUE_TYPE_STRING:
        {
            vector_t *tempVector = &(value->vector);
            cleanUpVector(tempVector);
            break;
        }
        case VALUE_TYPE_LIST:
        {
            vector_t *tempVector = &(value->vector);
            for (int64_t index = 0; index < tempVector->length; index++) {
                value_t *tempValue = findVectorElement(tempVector, index);
                removeValueReferenceHelper(tempValue, shouldRecur);
            }
            cleanUpVector(tempVector);
            break;
        }
        case VALUE_TYPE_FRAME:
        {
            valueList_t *tempValueList = &(value->frameVariableList);
            for (int32_t index = 0; index < tempValueList->length; index++) {
                value_t *tempValue = tempValueList->valueArray + index;
                removeValueReferenceHelper(tempValue, shouldRecur);
            }
            free(tempValueList->valueArray);
            break;
        }
        case VALUE_TYPE_CUSTOM_FUNCTION:
        {
            customFunctionHandle_t *tempHandle = value->customFunctionHandle;
            int32_t tempLength = tempHandle->function->scope.aliasVariableAmount;
            for (int32_t index = 0; index < tempLength; index++) {
                // TODO: Finish implementing.
                
            }
            free(tempHandle->aliasList);
            free(tempHandle);
            break;
        }
        default:
        {
            break;
        }
    }
    free(value);
}

// TODO: Decide whether alias values should be resolved.
int8_t valueIsInHeap(value_t *value) {
    int8_t tempType = value->type;
    return (tempType == VALUE_TYPE_STRING || tempType == VALUE_TYPE_LIST
            || tempType == VALUE_TYPE_FRAME || tempType == VALUE_TYPE_CUSTOM_FUNCTION);
}

void deleteHeapValueIfUnreferenced(heapValue_t *value) {
    // TODO: Update this to work with aliasCount.
    if (value->referenceCount <= 0 && value->lockDepth <= 0) {
        deleteHeapValue(value, true);
    }
}

void lockValue(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    value->heapValue->lockDepth += 1;
}

void unlockValue(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    heapValue_t *tempHeapValue = value->heapValue;
    tempHeapValue->lockDepth -= 1;
    deleteHeapValueIfUnreferenced(tempHeapValue);
}

void addValueReference(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    value->heapValue->referenceCount += 1;
}

void removeValueReferenceHelper(value_t *value, int8_t shouldRecur) {
    if (!valueIsInHeap(value)) {
        return;
    }
    heapValue_t *tempHeapValue = value->heapValue;
    tempHeapValue->referenceCount -= 1;
    if (shouldRecur) {
        deleteHeapValueIfUnreferenced(tempHeapValue);
    }
}

void removeValueReference(value_t *value) {
    removeValueReferenceHelper(value, true);
}

void swapValueReference(value_t *destination, value_t *source) {
    addValueReference(source);
    removeValueReference(destination);
    *destination = *source;
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
    switch (value.type) {
        case VALUE_TYPE_VOID:
        {
            return convertTextToStringValue((int8_t *)"(Void)");
        }
        case VALUE_TYPE_STRING:
        {
            if (shouldCopy) {
                return copyValue(value);
            } else {
                return value;
            }
        }
        case VALUE_TYPE_NUMBER:
        {
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
        case VALUE_TYPE_LIST:
        {
            vector_t tempVector;
            createEmptyVector(&tempVector, 1);
            int8_t tempCharacter;
            tempCharacter = '[';
            pushVectorElement(&tempVector, &tempCharacter);
            vector_t *tempList = &(value.heapValue->vector);
            for (int64_t index = 0; index < tempList->length; index++) {
                if (index > 0) {
                    tempCharacter = ',';
                    pushVectorElement(&tempVector, &tempCharacter);
                    tempCharacter = ' ';
                    pushVectorElement(&tempVector, &tempCharacter);
                }
                value_t tempValue;
                getVectorElement(&tempValue, tempList, index);
                value_t tempStringValue = convertValueToString(tempValue, false);
                vector_t *tempText = &(tempStringValue.heapValue->vector);
                pushVectorOntoVector(&tempVector, tempText);
                removeVectorElement(&tempVector, tempVector.length - 1);
            }
            tempCharacter = ']';
            pushVectorElement(&tempVector, &tempCharacter);
            tempCharacter = 0;
            pushVectorElement(&tempVector, &tempCharacter);
            return convertCharacterVectorToStringValue(&tempVector);
        }
        // TODO: Convert more values to strings.
        
        default: {
            break;
        }
    }
    value_t output;
    output.type = VALUE_TYPE_VOID;
    return output;
}

value_t readValueFromAlias(alias_t alias) {
    heapValue_t *heapValue = alias.container;
    int32_t index = (int32_t)(alias.index);
    if (heapValue->type == VALUE_TYPE_FRAME) {
        return heapValue->frameVariableList.valueArray[index];
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
        heapValue->frameVariableList.valueArray[index] = value;
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
    if (aliasValue.type != VALUE_TYPE_ALIAS) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected alias to value.");
        return;
    }
    writeValueToAlias(aliasValue.alias, value);
}


