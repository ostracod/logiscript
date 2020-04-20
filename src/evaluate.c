
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "evaluate.h"
#include "function.h"
#include "expression.h"
#include "operator.h"
#include "value.h"

aliasedValue_t evaluateExpression(heapValue_t *frame, baseExpression_t *expression) {
    aliasedValue_t output;
    output.value.type = VALUE_TYPE_VOID;
    output.alias.container = NULL;
    switch (expression->type) {
        case EXPRESSION_TYPE_CONSTANT:
        {
            constantExpression_t *constantExpression = (constantExpression_t *)expression;
            output.value = copyValue(constantExpression->value);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
        {
            variableExpression_t *variableExpression = (variableExpression_t *)expression;
            int32_t variableIndex = variableExpression->variable->scopeIndex;
            output = readFrameVariable(frame, variableIndex);
            break;
        }
        case EXPRESSION_TYPE_UNARY:
        {
            unaryExpression_t *unaryExpression = (unaryExpression_t *)expression;
            operator_t *tempOperator = unaryExpression->operator;
            aliasedValue_t tempOperand = evaluateExpression(
                frame,
                unaryExpression->operand
            );
            output = calculateUnaryOperator(tempOperator, tempOperand);
            break;
        }
        case EXPRESSION_TYPE_BINARY:
        {
            binaryExpression_t *binaryExpression = (binaryExpression_t *)expression;
            operator_t *tempOperator = binaryExpression->operator;
            aliasedValue_t tempOperand1 = evaluateExpression(
                frame,
                binaryExpression->operand1
            );
            aliasedValue_t tempOperand2 = evaluateExpression(
                frame,
                binaryExpression->operand2
            );
            output = calculateBinaryOperator(tempOperator, tempOperand1, tempOperand2);
            break;
        }
        case EXPRESSION_TYPE_FUNCTION:
        {
            heapValue_t *tempHandle = createFunctionHandle(
                ((customFunctionExpression_t *)expression)->customFunction
            );
            output.value = createValueFromHeapValue(tempHandle);
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
        value_t functionValue = evaluateExpression(
            frame,
            invocationStatement->function
        ).value;
        int32_t tempLength = (int32_t)(invocationStatement->argumentList.length);
        aliasedValue_t argumentList[tempLength];
        for (int32_t index = 0; index < tempLength; index++) {
            baseExpression_t *tempExpression;
            getVectorElement(&tempExpression, &(invocationStatement->argumentList), index);
            argumentList[index] = evaluateExpression(frame, tempExpression);
        }
        invokeFunction(functionValue, argumentList, tempLength);
    }
    // TODO: Evaluate other types of statements.
    
}

void evaluateScript(script_t *script) {
    heapValue_t *tempHandle = createFunctionHandle(script->topLevelFunction);
    script->globalFrame = invokeFunctionHandle(tempHandle, NULL, 0);
}


