
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "utilities.h"
#include "operator.h"
#include "error.h"

operator_t operatorSet[] = {
    {(int8_t *)"@", OPERATOR_DECLARE, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)".", OPERATOR_NAMESPACE, OPERATOR_ARRANGEMENT_BINARY, 2},
    {(int8_t *)"+", OPERATOR_ADD, OPERATOR_ARRANGEMENT_BINARY, 5},
    {(int8_t *)"-", OPERATOR_SUBTRACT, OPERATOR_ARRANGEMENT_BINARY, 5},
    {(int8_t *)"-", OPERATOR_NEGATE, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)"*", OPERATOR_MULTIPLY, OPERATOR_ARRANGEMENT_BINARY, 4},
    {(int8_t *)"/", OPERATOR_DIVIDE, OPERATOR_ARRANGEMENT_BINARY, 4},
    {(int8_t *)"%", OPERATOR_MODULUS, OPERATOR_ARRANGEMENT_BINARY, 4},
    {(int8_t *)"~", OPERATOR_BITWISE_NOT, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)"|", OPERATOR_BITWISE_OR, OPERATOR_ARRANGEMENT_BINARY, 11},
    {(int8_t *)"&", OPERATOR_BITWISE_AND, OPERATOR_ARRANGEMENT_BINARY, 9},
    {(int8_t *)"^", OPERATOR_BITWISE_XOR, OPERATOR_ARRANGEMENT_BINARY, 10},
    {(int8_t *)"<<", OPERATOR_BITSHIFT_LEFT, OPERATOR_ARRANGEMENT_BINARY, 6},
    {(int8_t *)">>", OPERATOR_BITSHIFT_RIGHT, OPERATOR_ARRANGEMENT_BINARY, 6},
    {(int8_t *)"!", OPERATOR_LOGICAL_NOT, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)"||", OPERATOR_LOGICAL_OR, OPERATOR_ARRANGEMENT_BINARY, 14},
    {(int8_t *)"&&", OPERATOR_LOGICAL_AND, OPERATOR_ARRANGEMENT_BINARY, 12},
    {(int8_t *)"^^", OPERATOR_LOGICAL_XOR, OPERATOR_ARRANGEMENT_BINARY, 13},
    {(int8_t *)"==", OPERATOR_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 8},
    {(int8_t *)"!=", OPERATOR_NOT_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 8},
    {(int8_t *)"===", OPERATOR_IDENTICAL, OPERATOR_ARRANGEMENT_BINARY, 8},
    {(int8_t *)"!==", OPERATOR_NOT_IDENTICAL, OPERATOR_ARRANGEMENT_BINARY, 8},
    {(int8_t *)">", OPERATOR_GREATER, OPERATOR_ARRANGEMENT_BINARY, 7},
    {(int8_t *)">=", OPERATOR_GREATER_OR_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 7},
    {(int8_t *)"<", OPERATOR_LESS, OPERATOR_ARRANGEMENT_BINARY, 7},
    {(int8_t *)"<=", OPERATOR_LESS_OR_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 7}
};

int8_t textMatchesOperator(int8_t *text, operator_t *operator) {
    int8_t tempOffset = 0;
    while (true) {
        int8_t tempCharacter1 = (operator->text)[tempOffset];
        int8_t tempCharacter2 = text[tempOffset];
        if (tempCharacter1 == 0) {
            return true;
        }
        if (tempCharacter1 != tempCharacter2) {
            return false;
        }
        tempOffset += 1;
    }
}

operator_t *getOperatorInText(int8_t *text, int8_t operatorArrangement) {
    int32_t operatorAmount = sizeof(operatorSet) / sizeof(*operatorSet);
    for (int8_t operatorLength = 3; operatorLength >= 1; operatorLength--) {
        for (int32_t index = 0; index < operatorAmount; index++) {
            operator_t *tempOperator = operatorSet + index;
            if (tempOperator->arrangement == operatorArrangement
                    && strlen((char *)(tempOperator->text)) == operatorLength
                    && textMatchesOperator(text, tempOperator)) {
                return tempOperator;
            }
        }
    }
    return NULL;
}

value_t calculateUnaryOperator(operator_t *operator, value_t operand) {
    value_t output;
    output.type = VALUE_TYPE_VOID;
    if (operand.type != VALUE_TYPE_NUMBER) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected number value.");
        return output;
    }
    double tempNumber = operand.numberValue;
    output.type = VALUE_TYPE_NUMBER;
    switch (operator->number) {
        case OPERATOR_NEGATE:
        {
            output.numberValue = -tempNumber;
            break;
        }
        case OPERATOR_BITWISE_NOT:
        {
            output.numberValue = ~(uint32_t)tempNumber;
            break;
        }
        case OPERATOR_LOGICAL_NOT:
        {
            output.numberValue = (tempNumber == 0);
            break;
        }
        default:
        {
            break;
        }
    }
    return output;
}

value_t concatenateSequences(value_t value1, value_t value2) {
    value_t output;
    output.type = VALUE_TYPE_VOID;
    if (value1.type == VALUE_TYPE_STRING && value2.type == VALUE_TYPE_STRING) {
        output = copyValue(value1);
        popVectorElement(NULL, &(output.heapValue->vector));
        pushVectorOntoVector(&(output.heapValue->vector), &(value2.heapValue->vector));
        return output;
    }
    if (value1.type == VALUE_TYPE_LIST && value2.type == VALUE_TYPE_LIST) {
        output = copyValue(value1);
        addValueReferencesInVector(&(value2.heapValue->vector));
        pushVectorOntoVector(&(output.heapValue->vector), &(value2.heapValue->vector));
        return output;
    }
    THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Invalid types for concatenation.");
    return output;
}

int8_t valuesAreIdentical(value_t value1, value_t value2);

int8_t valuesAreEqual(value_t value1, value_t value2) {
    if (value1.type != value2.type) {
        return false;
    }
    if (value1.type == VALUE_TYPE_NUMBER) {
        return (value1.numberValue == value2.numberValue);
    }
    if (value1.type == VALUE_TYPE_STRING) {
        vector_t *tempVector1 = &(value1.heapValue->vector);
        vector_t *tempVector2 = &(value2.heapValue->vector);
        return vectorsAreEqual(tempVector1, tempVector2);
    }
    if (value1.type == VALUE_TYPE_LIST) {
        vector_t *tempVector1 = &(value1.heapValue->vector);
        vector_t *tempVector2 = &(value2.heapValue->vector);
        if (tempVector1->length != tempVector2->length) {
            return false;
        }
        for (int64_t index = 0; index < tempVector1->length; index++) {
            value_t tempValue1;
            value_t tempValue2;
            getVectorElement(&tempValue1, tempVector1, index);
            getVectorElement(&tempValue2, tempVector2, index);
            if (!valuesAreIdentical(tempValue1, tempValue2)) {
                return false;
            }
        }
        return true;
    }
    if (value1.type == VALUE_TYPE_BUILT_IN_FUNCTION) {
        return (value1.builtInFunction == value2.builtInFunction);
    }
    if (value1.type == VALUE_TYPE_CUSTOM_FUNCTION) {
        customFunctionHandle_t *tempHandle1 = value1.heapValue->customFunctionHandle;
        customFunctionHandle_t *tempHandle2 = value2.heapValue->customFunctionHandle;
        return (tempHandle1->function == tempHandle2->function);
    }
    return true;
}

int8_t valuesAreIdentical(value_t value1, value_t value2) {
    if (value1.type != value2.type) {
        return false;
    }
    if (value1.type == VALUE_TYPE_STRING || value1.type == VALUE_TYPE_LIST
            || value1.type == VALUE_TYPE_CUSTOM_FUNCTION) {
        return (value1.heapValue == value2.heapValue);
    }
    return valuesAreEqual(value1, value2);
}

value_t calculateBinaryOperator(
    operator_t *operator,
    value_t operand1,
    value_t operand2
) {
    value_t output;
    output.type = VALUE_TYPE_VOID;
    int8_t hasProcessedOperator = true;
    switch (operator->number) {
        case OPERATOR_ADD:
        {
            if (operand1.type == VALUE_TYPE_NUMBER && operand2.type == VALUE_TYPE_NUMBER) {
                output.type = VALUE_TYPE_NUMBER;
                output.numberValue = operand1.numberValue + operand2.numberValue;
            } else {
                output = concatenateSequences(operand1, operand2);
            }
            break;
        }
        case OPERATOR_EQUAL:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = valuesAreEqual(operand1, operand2);
            break;
        }
        case OPERATOR_NOT_EQUAL:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = !valuesAreEqual(operand1, operand2);
            break;
        }
        case OPERATOR_IDENTICAL:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = valuesAreIdentical(operand1, operand2);
            break;
        }
        case OPERATOR_NOT_IDENTICAL:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = !valuesAreIdentical(operand1, operand2);
            break;
        }
        default:
        {
            hasProcessedOperator = false;
            break;
        }
    }
    if (hasProcessedOperator) {
        return output;
    }
    if (operand1.type != VALUE_TYPE_NUMBER || operand2.type != VALUE_TYPE_NUMBER) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected number value.");
        return output;
    }
    double tempNumber1 = operand1.numberValue;
    double tempNumber2 = operand2.numberValue;
    output.type = VALUE_TYPE_NUMBER;
    switch (operator->number) {
        case OPERATOR_SUBTRACT:
        {
            output.numberValue = tempNumber1 - tempNumber2;
            break;
        }
        case OPERATOR_MULTIPLY:
        {
            output.numberValue = tempNumber1 * tempNumber2;
            break;
        }
        case OPERATOR_DIVIDE:
        {
            if (tempNumber2 == 0) {
                THROW_BUILT_IN_ERROR(NUMBER_ERROR_CONSTANT, "Division by zero.");
                return output;
            }
            output.numberValue = tempNumber1 / tempNumber2;
            break;
        }
        case OPERATOR_MODULUS:
        {
            if (tempNumber2 == 0) {
                THROW_BUILT_IN_ERROR(NUMBER_ERROR_CONSTANT, "Division by zero.");
                return output;
            }
            output.numberValue = fmod(tempNumber1, tempNumber2);
            break;
        }
        case OPERATOR_BITWISE_OR:
        {
            output.numberValue = (uint32_t)tempNumber1 | (uint32_t)tempNumber2;
            break;
        }
        case OPERATOR_BITWISE_AND:
        {
            output.numberValue = (uint32_t)tempNumber1 & (uint32_t)tempNumber2;
            break;
        }
        case OPERATOR_BITWISE_XOR:
        {
            output.numberValue = (uint32_t)tempNumber1 ^ (uint32_t)tempNumber2;
            break;
        }
        case OPERATOR_BITSHIFT_LEFT:
        {
            output.numberValue = (uint32_t)tempNumber1 << (uint32_t)tempNumber2;
            break;
        }
        case OPERATOR_BITSHIFT_RIGHT:
        {
            output.numberValue = (uint32_t)tempNumber1 >> (uint32_t)tempNumber2;
            break;
        }
        case OPERATOR_LOGICAL_OR:
        {
            output.numberValue = ((tempNumber1 != 0) || (tempNumber2 != 0));
            break;
        }
        case OPERATOR_LOGICAL_AND:
        {
            output.numberValue = ((tempNumber1 != 0) && (tempNumber2 != 0));
            break;
        }
        case OPERATOR_LOGICAL_XOR:
        {
            output.numberValue = ((tempNumber1 != 0) != (tempNumber2 != 0));
            break;
        }
        case OPERATOR_GREATER:
        {
            output.numberValue = (tempNumber1 > tempNumber2);
            break;
        }
        case OPERATOR_GREATER_OR_EQUAL:
        {
            output.numberValue = (tempNumber1 >= tempNumber2);
            break;
        }
        case OPERATOR_LESS:
        {
            output.numberValue = (tempNumber1 < tempNumber2);
            break;
        }
        case OPERATOR_LESS_OR_EQUAL:
        {
            output.numberValue = (tempNumber1 <= tempNumber2);
            break;
        }
        default:
        {
            break;
        }
    }
    return output;
}


