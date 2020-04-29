
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

int32_t markAndSweepDelay = 0;

value_t evaluateAndResolveExpression(heapValue_t *frame, baseExpression_t *expression);

// Output will be locked.
hyperValue_t evaluateExpression(heapValue_t *frame, baseExpression_t *expression) {
    hyperValue_t output;
    output.type = HYPER_VALUE_TYPE_VALUE;
    output.value.type = VALUE_TYPE_VOID;
    switch (expression->type) {
        case EXPRESSION_TYPE_CONSTANT:
        {
            constantExpression_t *constantExpression = (constantExpression_t *)expression;
            output.value = copyValue(constantExpression->value);
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
                value_t tempValue = evaluateAndResolveExpression(
                    frame,
                    tempElementExpression
                );
                if (hasThrownError) {
                    unlockValuesInVector(&tempValueList);
                    cleanUpVector(&tempValueList);
                    return output;
                }
                pushVectorElement(&tempValueList, &tempValue);
            }
            heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_LIST);
            tempHeapValue->vector = tempValueList;
            addValueReferencesInVector(&tempValueList);
            unlockValuesInVector(&tempValueList);
            output.value = createValueFromHeapValue(tempHeapValue);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
        {
            variableExpression_t *variableExpression = (variableExpression_t *)expression;
            int32_t tempScopeIndex = variableExpression->variable->scopeIndex;
            output.type = HYPER_VALUE_TYPE_ALIAS;
            output.alias = getAliasToFrameVariable(frame, tempScopeIndex);
            break;
        }
        case EXPRESSION_TYPE_UNARY:
        {
            unaryExpression_t *unaryExpression = (unaryExpression_t *)expression;
            operator_t *tempOperator = unaryExpression->operator;
            value_t tempOperand = evaluateAndResolveExpression(
                frame,
                unaryExpression->operand
            );
            if (hasThrownError) {
                return output;
            }
            output.value = calculateUnaryOperator(tempOperator, tempOperand);
            unlockValue(&tempOperand);
            break;
        }
        case EXPRESSION_TYPE_BINARY:
        {
            binaryExpression_t *binaryExpression = (binaryExpression_t *)expression;
            operator_t *tempOperator = binaryExpression->operator;
            value_t tempOperand1 = evaluateAndResolveExpression(
                frame,
                binaryExpression->operand1
            );
            if (hasThrownError) {
                return output;
            }
            value_t tempOperand2 = evaluateAndResolveExpression(
                frame,
                binaryExpression->operand2
            );
            if (hasThrownError) {
                unlockValue(&tempOperand1);
                return output;
            }
            output.value = calculateBinaryOperator(tempOperator, tempOperand1, tempOperand2);
            unlockValue(&tempOperand1);
            unlockValue(&tempOperand2);
            break;
        }
        case EXPRESSION_TYPE_FUNCTION:
        {
            heapValue_t *tempHandle = createFunctionHandle(
                frame,
                ((customFunctionExpression_t *)expression)->customFunction
            );
            output.value = createValueFromHeapValue(tempHandle);
            break;
        }
        // TODO: Evaluate other types of expressions.
        
        default:
        {
            break;
        }
    }
    lockHyperValue(&output);
    return output;
}

value_t evaluateAndResolveExpression(heapValue_t *frame, baseExpression_t *expression) {
    hyperValue_t tempHyperValue = evaluateExpression(frame, expression);
    if (hasThrownError) {
        value_t output;
        output.type = VALUE_TYPE_VOID;
        return output;
    }
    value_t output = resolveAliasValue(tempHyperValue);
    lockValue(&output);
    unlockHyperValue(&tempHyperValue);
    return output;
}

void evaluateStatement(heapValue_t *frame, baseStatement_t *statement) {
    if (statement->type == STATEMENT_TYPE_INVOCATION) {
        invocationStatement_t *invocationStatement = (invocationStatement_t *)statement;
        int32_t tempLength = (int32_t)(invocationStatement->argumentList.length);
        hyperValue_t tempValueArray[tempLength];
        hyperValueList_t argumentList;
        argumentList.valueArray = tempValueArray;
        argumentList.length = tempLength;
        for (int32_t index = 0; index < tempLength; index++) {
            hyperValue_t *tempValue = tempValueArray + index;
            tempValue->type = HYPER_VALUE_TYPE_VALUE;
            tempValue->value.type = VALUE_TYPE_VOID;
        }
        value_t functionValue = evaluateAndResolveExpression(
            frame,
            invocationStatement->function
        );
        if (hasThrownError) {
            unlockFunctionInvocationValues(&functionValue, &argumentList);
            return;
        }
        for (int32_t index = 0; index < tempLength; index++) {
            baseExpression_t *tempExpression;
            getVectorElement(&tempExpression, &(invocationStatement->argumentList), index);
            tempValueArray[index] = evaluateExpression(frame, tempExpression);
            if (hasThrownError) {
                unlockFunctionInvocationValues(&functionValue, &argumentList);
                return;
            }
        }
        invokeFunction(functionValue, &argumentList);
        unlockFunctionInvocationValues(&functionValue, &argumentList);
    }
    // TODO: Evaluate other types of statements.
    
    markAndSweepDelay += 1;
    if (markAndSweepDelay > 10000) {
        markAndSweepHeapValues();
        markAndSweepDelay = 0;
    }
}

void evaluateScript(script_t *script) {
    heapValue_t *tempHandle = createFunctionHandle(
        NULL,
        script->topLevelFunction
    );
    script->globalFrame = invokeFunctionHandle(tempHandle, NULL);
}


