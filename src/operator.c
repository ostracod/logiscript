
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "utilities.h"
#include "operator.h"

operator_t operatorSet[] = {
    {(int8_t *)"@", OPERATOR_DECLARE, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)".", OPERATOR_NAMESPACE, OPERATOR_ARRANGEMENT_BINARY, 2},
    {(int8_t *)"+", OPERATOR_ADD, OPERATOR_ARRANGEMENT_BINARY, 4},
    {(int8_t *)"-", OPERATOR_SUBTRACT, OPERATOR_ARRANGEMENT_BINARY, 4},
    {(int8_t *)"-", OPERATOR_NEGATE, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)"*", OPERATOR_MULTIPLY, OPERATOR_ARRANGEMENT_BINARY, 3},
    {(int8_t *)"/", OPERATOR_DIVIDE, OPERATOR_ARRANGEMENT_BINARY, 3},
    {(int8_t *)"%", OPERATOR_MODULUS, OPERATOR_ARRANGEMENT_BINARY, 3},
    {(int8_t *)"~", OPERATOR_BITWISE_NOT, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)"|", OPERATOR_BITWISE_OR, OPERATOR_ARRANGEMENT_BINARY, 10},
    {(int8_t *)"&", OPERATOR_BITWISE_AND, OPERATOR_ARRANGEMENT_BINARY, 8},
    {(int8_t *)"^", OPERATOR_BITWISE_XOR, OPERATOR_ARRANGEMENT_BINARY, 9},
    {(int8_t *)"<<", OPERATOR_BITSHIFT_LEFT, OPERATOR_ARRANGEMENT_BINARY, 5},
    {(int8_t *)">>", OPERATOR_BITSHIFT_RIGHT, OPERATOR_ARRANGEMENT_BINARY, 5},
    {(int8_t *)"!", OPERATOR_LOGICAL_NOT, OPERATOR_ARRANGEMENT_UNARY, 1},
    {(int8_t *)"||", OPERATOR_LOGICAL_OR, OPERATOR_ARRANGEMENT_BINARY, 13},
    {(int8_t *)"&&", OPERATOR_LOGICAL_AND, OPERATOR_ARRANGEMENT_BINARY, 11},
    {(int8_t *)"^^", OPERATOR_LOGICAL_XOR, OPERATOR_ARRANGEMENT_BINARY, 12},
    {(int8_t *)"==", OPERATOR_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 7},
    {(int8_t *)"!=", OPERATOR_NOT_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 7},
    {(int8_t *)"===", OPERATOR_IDENTICAL, OPERATOR_ARRANGEMENT_BINARY, 7},
    {(int8_t *)"!==", OPERATOR_NOT_IDENTICAL, OPERATOR_ARRANGEMENT_BINARY, 7},
    {(int8_t *)">", OPERATOR_GREATER, OPERATOR_ARRANGEMENT_BINARY, 6},
    {(int8_t *)">=", OPERATOR_GREATER_OR_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 6},
    {(int8_t *)"<", OPERATOR_LESS, OPERATOR_ARRANGEMENT_BINARY, 6},
    {(int8_t *)"<=", OPERATOR_LESS_OR_EQUAL, OPERATOR_ARRANGEMENT_BINARY, 6}
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
    operand = resolveAliasValue(operand);
    double tempNumber = operand.numberValue;
    switch (operator->number) {
        case OPERATOR_NEGATE:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = -tempNumber;
            break;
        }
        // TODO: Calculate with more unary operators.
        
        default:
        {
            break;
        }
    }
    return output;
}

value_t calculateBinaryOperator(
    operator_t *operator,
    value_t operand1,
    value_t operand2
) {
    value_t output;
    output.type = VALUE_TYPE_VOID;
    operand1 = resolveAliasValue(operand1);
    operand2 = resolveAliasValue(operand2);
    double tempNumber1 = operand1.numberValue;
    double tempNumber2 = operand2.numberValue;
    switch (operator->number) {
        case OPERATOR_ADD:
        {
            // TODO: Support concatenation.
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = tempNumber1 + tempNumber2;
            break;
        }
        case OPERATOR_SUBTRACT:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = tempNumber1 - tempNumber2;
            break;
        }
        case OPERATOR_MULTIPLY:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = tempNumber1 * tempNumber2;
            break;
        }
        case OPERATOR_DIVIDE:
        {
            // TODO: Check for division by zero.
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = tempNumber1 / tempNumber2;
            break;
        }
        case OPERATOR_MODULUS:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = fmod(tempNumber1, tempNumber2);
            break;
        }
        case OPERATOR_EQUAL:
        {
            // TODO: Test equality of strings and lists.
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = (tempNumber1 == tempNumber2);
            break;
        }
        case OPERATOR_NOT_EQUAL:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = (tempNumber1 != tempNumber2);
            break;
        }
        case OPERATOR_GREATER:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = (tempNumber1 > tempNumber2);
            break;
        }
        case OPERATOR_GREATER_OR_EQUAL:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = (tempNumber1 >= tempNumber2);
            break;
        }
        case OPERATOR_LESS:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = (tempNumber1 < tempNumber2);
            break;
        }
        case OPERATOR_LESS_OR_EQUAL:
        {
            output.type = VALUE_TYPE_NUMBER;
            output.numberValue = (tempNumber1 <= tempNumber2);
            break;
        }
        // TODO: Calculate with more unary operators.
        
        default:
        {
            break;
        }
    }
    return output;
}


