
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "evaluate.h"
#include "function.h"
#include "expression.h"
#include "operator.h"
#include "value.h"
#include "error.h"

value_t evaluateExpression(heapValue_t *frame, baseExpression_t *expression) {
    value_t output;
    output.type = VALUE_TYPE_VOID;
    switch (expression->type) {
        case EXPRESSION_TYPE_CONSTANT:
        {
            constantExpression_t *constantExpression = (constantExpression_t *)expression;
            output = copyValue(constantExpression->value);
            break;
        }
        case EXPRESSION_TYPE_LIST:
        {
            listExpression_t *listExpression = (listExpression_t *)expression;
            vector_t tempValueList;
            createEmptyVector(&tempValueList, sizeof(value_t));
            for (int64_t index = 0; index < listExpression->expressionList.length; index++) {
                baseExpression_t *tempElementExpression;
                getVectorElement(
                    &tempElementExpression,
                    &(listExpression->expressionList),
                    index
                );
                value_t tempValue = evaluateExpression(frame, tempElementExpression);
                if (hasThrownError) {
                    return output;
                }
                tempValue = resolveAliasValue(tempValue);
                pushVectorElement(&tempValueList, &tempValue);
            }
            heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_LIST);
            tempHeapValue->vector = tempValueList;
            output = createValueFromHeapValue(tempHeapValue);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
        {
            variableExpression_t *variableExpression = (variableExpression_t *)expression;
            int32_t tempScopeIndex = variableExpression->variable->scopeIndex;
            output.type = VALUE_TYPE_ALIAS;
            output.alias = getAliasToFrameVariable(frame, tempScopeIndex);
            break;
        }
        case EXPRESSION_TYPE_UNARY:
        {
            unaryExpression_t *unaryExpression = (unaryExpression_t *)expression;
            operator_t *tempOperator = unaryExpression->operator;
            value_t tempOperand = evaluateExpression(frame, unaryExpression->operand);
            if (hasThrownError) {
                return output;
            }
            output = calculateUnaryOperator(tempOperator, tempOperand);
            break;
        }
        case EXPRESSION_TYPE_BINARY:
        {
            binaryExpression_t *binaryExpression = (binaryExpression_t *)expression;
            operator_t *tempOperator = binaryExpression->operator;
            value_t tempOperand1 = evaluateExpression(frame, binaryExpression->operand1);
            if (hasThrownError) {
                return output;
            }
            value_t tempOperand2 = evaluateExpression(frame, binaryExpression->operand2);
            if (hasThrownError) {
                return output;
            }
            output = calculateBinaryOperator(tempOperator, tempOperand1, tempOperand2);
            break;
        }
        case EXPRESSION_TYPE_FUNCTION:
        {
            heapValue_t *tempHandle = createFunctionHandle(
                frame,
                ((customFunctionExpression_t *)expression)->customFunction
            );
            output = createValueFromHeapValue(tempHandle);
            break;
        }
        // TODO: Evaluate other types of expressions.
        
        default:
        {
            break;
        }
    }
    return output;
}

void evaluateStatement(heapValue_t *frame, baseStatement_t *statement) {
    if (statement->type == STATEMENT_TYPE_INVOCATION) {
        invocationStatement_t *invocationStatement = (invocationStatement_t *)statement;
        value_t functionValue = evaluateExpression(frame, invocationStatement->function);
        if (hasThrownError) {
            return;
        }
        functionValue = resolveAliasValue(functionValue);
        int32_t tempLength = (int32_t)(invocationStatement->argumentList.length);
        value_t tempValueArray[tempLength];
        for (int32_t index = 0; index < tempLength; index++) {
            baseExpression_t *tempExpression;
            getVectorElement(&tempExpression, &(invocationStatement->argumentList), index);
            tempValueArray[index] = evaluateExpression(frame, tempExpression);
            if (hasThrownError) {
                return;
            }
        }
        valueList_t argumentList;
        argumentList.valueArray = tempValueArray;
        argumentList.length = tempLength;
        invokeFunction(functionValue, &argumentList);
    }
    // TODO: Evaluate other types of statements.
    
}

void evaluateScript(script_t *script) {
    heapValue_t *tempHandle = createFunctionHandle(
        NULL,
        script->topLevelFunction
    );
    script->globalFrame = invokeFunctionHandle(tempHandle, NULL);
}


