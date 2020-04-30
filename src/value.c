
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
    firstHeapValue = output;
    return output;
}

void removeHeapValueReferenceHelper(heapValue_t *heapValue, int8_t shouldRecur);
void removeValueReferenceHelper(value_t *value, int8_t shouldRecur);
void removeHyperValueReferenceHelper(hyperValue_t *hyperValue, int8_t shouldRecur);

void removeNestedHeapValueReferences(heapValue_t *heapValue, int8_t shouldRecur) {
    switch (heapValue->type) {
        case VALUE_TYPE_LIST:
        {
            vector_t *tempVector = &(heapValue->vector);
            for (int64_t index = 0; index < tempVector->length; index++) {
                value_t *tempValue = findVectorElement(tempVector, index);
                removeValueReferenceHelper(tempValue, shouldRecur);
            }
            break;
        }
        case VALUE_TYPE_FRAME:
        {
            hyperValueList_t *tempValueList = &(heapValue->frameVariableList);
            for (int32_t index = 0; index < tempValueList->length; index++) {
                hyperValue_t *tempValue = tempValueList->valueArray + index;
                removeHyperValueReferenceHelper(tempValue, shouldRecur);
            }
            break;
        }
        case VALUE_TYPE_CUSTOM_FUNCTION:
        {
            customFunctionHandle_t *tempHandle = heapValue->customFunctionHandle;
            int32_t tempLength = tempHandle->function->scope.aliasVariableAmount;
            for (int32_t index = 0; index < tempLength; index++) {
                alias_t *tempAlias = tempHandle->aliasList + index;
                removeHeapValueReferenceHelper(tempAlias->container, shouldRecur);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void deleteHeapValue(heapValue_t *heapValue, int8_t shouldRecur) {
    if (heapValue->previous == NULL) {
        firstHeapValue = heapValue->next;
    } else {
        heapValue->previous->next = heapValue->next;
    }
    if (heapValue->next != NULL) {
        heapValue->next->previous = heapValue->previous;
    }
    if (shouldRecur) {
        removeNestedHeapValueReferences(heapValue, shouldRecur);
    }
    switch (heapValue->type) {
        case VALUE_TYPE_STRING:
        case VALUE_TYPE_LIST:
        {
            cleanUpVector(&(heapValue->vector));
            break;
        }
        case VALUE_TYPE_FRAME:
        {
            hyperValueList_t *tempValueList = &(heapValue->frameVariableList);
            free(tempValueList->valueArray);
            break;
        }
        case VALUE_TYPE_CUSTOM_FUNCTION:
        {
            customFunctionHandle_t *tempHandle = heapValue->customFunctionHandle;
            if (tempHandle->aliasList != NULL) {
                free(tempHandle->aliasList);
            }
            free(tempHandle);
            break;
        }
        default:
        {
            break;
        }
    }
    free(heapValue);
}

int8_t valueIsInHeap(value_t *value) {
    int8_t tempType = value->type;
    return (tempType == VALUE_TYPE_STRING || tempType == VALUE_TYPE_LIST
            || tempType == VALUE_TYPE_FRAME || tempType == VALUE_TYPE_CUSTOM_FUNCTION);
}

void deleteHeapValueIfUnreferenced(heapValue_t *heapValue) {
    if (heapValue->referenceCount <= 0 && heapValue->lockDepth <= 0) {
        deleteHeapValue(heapValue, true);
    }
}

void deleteValueIfUnreferenced(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    deleteHeapValueIfUnreferenced(value->heapValue);
}

void lockHeapValue(heapValue_t *heapValue) {
    heapValue->lockDepth += 1;
}

void lockValue(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    lockHeapValue(value->heapValue);
}

void lockHyperValue(hyperValue_t *hyperValue) {
    if (hyperValue->type == HYPER_VALUE_TYPE_ALIAS) {
        lockHeapValue(hyperValue->alias.container);
    } else {
        lockValue(&(hyperValue->value));
    }
}

void unlockHeapValue(heapValue_t *heapValue) {
    heapValue->lockDepth -= 1;
    deleteHeapValueIfUnreferenced(heapValue);
}

void unlockValue(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    unlockHeapValue(value->heapValue);
}

void unlockHyperValue(hyperValue_t *hyperValue) {
    if (hyperValue->type == HYPER_VALUE_TYPE_ALIAS) {
        unlockHeapValue(hyperValue->alias.container);
    } else {
        unlockValue(&(hyperValue->value));
    }
}

void unlockFunctionInvocationValues(value_t *function, hyperValueList_t *hyperValueList) {
    unlockValue(function);
    for (int32_t index = 0; index < hyperValueList->length; index++) {
        unlockHyperValue(hyperValueList->valueArray + index);
    }
}

void unlockValuesInVector(vector_t *vector) {
    for (int64_t index = 0; index < vector->length; index++) {
        value_t *tempValue = findVectorElement(vector, index);
        unlockValue(tempValue);
    }
}

void addHeapValueReference(heapValue_t *heapValue) {
    heapValue->referenceCount += 1;
}

void addValueReference(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    addHeapValueReference(value->heapValue);
}

void addHyperValueReference(hyperValue_t *hyperValue) {
    if (hyperValue->type == HYPER_VALUE_TYPE_ALIAS) {
        addHeapValueReference(hyperValue->alias.container);
    } else {
        addValueReference(&(hyperValue->value));
    }
}

void addValueReferencesInVector(vector_t *vector) {
    for (int64_t index = 0; index < vector->length; index++) {
        value_t *tempValue = findVectorElement(vector, index);
        addValueReference(tempValue);
    }
}

void removeHeapValueReferenceHelper(heapValue_t *heapValue, int8_t shouldRecur) {
    heapValue->referenceCount -= 1;
    if (shouldRecur) {
        deleteHeapValueIfUnreferenced(heapValue);
    }
}

void removeValueReferenceHelper(value_t *value, int8_t shouldRecur) {
    if (!valueIsInHeap(value)) {
        return;
    }
    removeHeapValueReferenceHelper(value->heapValue, shouldRecur);
}

void removeHyperValueReferenceHelper(hyperValue_t *hyperValue, int8_t shouldRecur) {
    if (hyperValue->type == HYPER_VALUE_TYPE_ALIAS) {
        removeHeapValueReferenceHelper(hyperValue->alias.container, shouldRecur);
    } else {
        removeValueReferenceHelper(&(hyperValue->value), shouldRecur);
    }
}

void removeHeapValueReference(heapValue_t *heapValue) {
    removeHeapValueReferenceHelper(heapValue, true);
}

void removeValueReference(value_t *value) {
    removeValueReferenceHelper(value, true);
}

void removeHyperValueReference(hyperValue_t *hyperValue) {
    removeHyperValueReferenceHelper(hyperValue, true);
}

void swapValueReference(value_t *destination, value_t *source) {
    addValueReference(source);
    removeValueReference(destination);
    *destination = *source;
}

void swapHyperValueReference(hyperValue_t *destination, hyperValue_t *source) {
    addHyperValueReference(source);
    removeHyperValueReference(destination);
    *destination = *source;
}

void markValue(value_t *value);
void markAlias(alias_t *alias);
void markHyperValue(hyperValue_t *hyperValue);

void markHeapValue(heapValue_t *heapValue, int8_t mark) {
    if (mark == HEAP_VALUE_MARK_WEAK) {
        if (heapValue->mark == HEAP_VALUE_MARK_NONE) {
            heapValue->mark = HEAP_VALUE_MARK_WEAK;
        }
        return;
    }
    if (mark == HEAP_VALUE_MARK_STRONG) {
        if (heapValue->mark == HEAP_VALUE_MARK_STRONG) {
            return;
        }
        heapValue->mark = HEAP_VALUE_MARK_STRONG;
    }
    switch (heapValue->type) {
        case VALUE_TYPE_LIST:
        {
            vector_t *tempVector = &(heapValue->vector);
            for (int64_t index = 0; index < tempVector->length; index++) {
                value_t *tempValue = findVectorElement(tempVector, index);
                markValue(tempValue);
            }
            break;
        }
        case VALUE_TYPE_FRAME:
        {
            hyperValueList_t *tempValueList = &(heapValue->frameVariableList);
            for (int32_t index = 0; index < tempValueList->length; index++) {
                hyperValue_t *tempValue = tempValueList->valueArray + index;
                markHyperValue(tempValue);
            }
            break;
        }
        case VALUE_TYPE_CUSTOM_FUNCTION:
        {
            customFunctionHandle_t *tempHandle = heapValue->customFunctionHandle;
            int32_t tempLength = tempHandle->function->scope.aliasVariableAmount;
            for (int32_t index = 0; index < tempLength; index++) {
                alias_t *tempAlias = tempHandle->aliasList + index;
                markAlias(tempAlias);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void markValue(value_t *value) {
    if (!valueIsInHeap(value)) {
        return;
    }
    markHeapValue(value->heapValue, HEAP_VALUE_MARK_STRONG);
}

void markAlias(alias_t *alias) {
    markHeapValue(alias->container, HEAP_VALUE_MARK_WEAK);
    value_t tempValue = readValueFromAlias(*alias);
    markValue(&tempValue);
}

void markHyperValue(hyperValue_t *hyperValue) {
    if (hyperValue->type == HYPER_VALUE_TYPE_ALIAS) {
        markAlias(&(hyperValue->alias));
    } else {
        markValue(&(hyperValue->value));
    }
}

void markAndSweepHeapValues() {
    // Unmark all values.
    heapValue_t *tempHeapValue = firstHeapValue;
    while (tempHeapValue != NULL) {
        tempHeapValue->mark = HEAP_VALUE_MARK_NONE;
        tempHeapValue = tempHeapValue->next;
    }
    // Mark all locked values.
    tempHeapValue = firstHeapValue;
    while (tempHeapValue != NULL) {
        if (tempHeapValue->lockDepth > 0) {
            markHeapValue(tempHeapValue, HEAP_VALUE_MARK_STRONG);
        }
        tempHeapValue = tempHeapValue->next;
    }
    // Remove references of all unmarked values.
    tempHeapValue = firstHeapValue;
    while (tempHeapValue != NULL) {
        if (tempHeapValue->mark == HEAP_VALUE_MARK_NONE) {
            removeNestedHeapValueReferences(tempHeapValue, false);
        }
        tempHeapValue = tempHeapValue->next;
    }
    // Delete all unmarked values.
    tempHeapValue = firstHeapValue;
    while (tempHeapValue != NULL) {
        heapValue_t *nextHeapValue = tempHeapValue->next;
        if (tempHeapValue->mark == HEAP_VALUE_MARK_NONE) {
            deleteHeapValue(tempHeapValue, false);
        }
        tempHeapValue = nextHeapValue;
    }
}

value_t createValueFromHeapValue(heapValue_t *heapValue) {
    value_t output;
    output.type = heapValue->type;
    output.heapValue = heapValue;
    return output;
}

value_t copyValue(value_t value) {
    if (value.type == VALUE_TYPE_STRING) {
        heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_STRING);
        copyVector(&(tempHeapValue->vector), &(value.heapValue->vector));
        return createValueFromHeapValue(tempHeapValue);
    }
    if (value.type == VALUE_TYPE_LIST) {
        heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_LIST);
        copyVector(&(tempHeapValue->vector), &(value.heapValue->vector));
        addValueReferencesInVector(&(tempHeapValue->vector));
        return createValueFromHeapValue(tempHeapValue);
    }
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
                deleteValueIfUnreferenced(&tempStringValue);
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
    value_t output;
    output.type = VALUE_TYPE_VOID;
    heapValue_t *heapValue = alias.container;
    int32_t index = (int32_t)(alias.index);
    if (heapValue->type == VALUE_TYPE_FRAME) {
        hyperValue_t *tempValue = heapValue->frameVariableList.valueArray + index;
        // Note: tempValue cannot have type HYPER_VALUE_TYPE_ALIAS,
        // because this is an invariant of alias_t.
        return tempValue->value;
    }
    if (heapValue->type == VALUE_TYPE_LIST) {
        if (alias.index < 0 || alias.index >= heapValue->vector.length) {
            THROW_BUILT_IN_ERROR(NUMBER_ERROR_CONSTANT, "Index is out of bounds.");
            return output;
        }
        value_t output;
        getVectorElement(&output, &(heapValue->vector), alias.index);
        return output;
    }
    if (heapValue->type == VALUE_TYPE_STRING) {
        if (alias.index < 0 || alias.index >= heapValue->vector.length - 1) {
            THROW_BUILT_IN_ERROR(NUMBER_ERROR_CONSTANT, "Index is out of bounds.");
            return output;
        }
        int8_t tempCharacter;
        getVectorElement(&tempCharacter, &(heapValue->vector), alias.index);
        output.type = VALUE_TYPE_NUMBER;
        output.numberValue = tempCharacter;
        return output;
    }
    return output;
}

void writeValueToAlias(alias_t alias, value_t value) {
    heapValue_t *heapValue = alias.container;
    int32_t index = (int32_t)(alias.index);
    if (heapValue->type == VALUE_TYPE_FRAME) {
        hyperValue_t *tempValue = heapValue->frameVariableList.valueArray + index;
        // Note: tempValue cannot have type HYPER_VALUE_TYPE_ALIAS,
        // because this is an invariant of alias_t.
        swapValueReference(&(tempValue->value), &value);
    }
    if (heapValue->type == VALUE_TYPE_LIST) {
        if (alias.index < 0 || alias.index >= heapValue->vector.length) {
            THROW_BUILT_IN_ERROR(NUMBER_ERROR_CONSTANT, "Index is out of bounds.");
            return;
        }
        value_t *tempValue = findVectorElement(&(heapValue->vector), alias.index);
        swapValueReference(tempValue, &value);
    }
    if (heapValue->type == VALUE_TYPE_STRING) {
        if (alias.index < 0 || alias.index >= heapValue->vector.length - 1) {
            THROW_BUILT_IN_ERROR(NUMBER_ERROR_CONSTANT, "Index is out of bounds.");
            return;
        }
        if (value.type != VALUE_TYPE_NUMBER) {
            THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected number value.");
            return;
        }
        int8_t tempCharacter = (int8_t)(value.numberValue);
        setVectorElement(&(heapValue->vector), alias.index, &tempCharacter);
    }
}

value_t resolveAliasValue(hyperValue_t hyperValue) {
    if (hyperValue.type == HYPER_VALUE_TYPE_ALIAS) {
        return readValueFromAlias(hyperValue.alias);
    }
    return hyperValue.value;
}

void writeValueToAliasValue(hyperValue_t aliasValue, value_t value) {
    if (aliasValue.type != HYPER_VALUE_TYPE_ALIAS) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected alias to value.");
        return;
    }
    writeValueToAlias(aliasValue.alias, value);
}

void printAllHeapValues() {
    printf("    HEAP VALUES:\n");
    heapValue_t *tempHeapValue = firstHeapValue;
    while (tempHeapValue != NULL) {
        printf(
            "    Type = %d; Lock depth = %d; Reference count = %lld\n",
            tempHeapValue->type,
            tempHeapValue->lockDepth,
            tempHeapValue->referenceCount
        );
        tempHeapValue = tempHeapValue->next;
    }
}


