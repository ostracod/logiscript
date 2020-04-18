
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "evaluate.h"
#include "function.h"
#include "expression.h"
#include "operator.h"
#include "value.h"

aliasedValue_t evaluateExpression(baseExpression_t *expression) {
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
        case EXPRESSION_TYPE_UNARY:
        {
            unaryExpression_t *unaryExpression = (unaryExpression_t *)expression;
            operator_t *tempOperator = unaryExpression->operator;
            aliasedValue_t tempOperand = evaluateExpression(unaryExpression->operand);
            output = calculateUnaryOperator(tempOperator, tempOperand);
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

void evaluateStatement(baseStatement_t *statement) {
    if (statement->type == STATEMENT_TYPE_INVOCATION) {
        invocationStatement_t *invocationStatement = (invocationStatement_t *)statement;
        value_t functionValue = evaluateExpression(invocationStatement->function).value;
        int32_t tempLength = (int32_t)(invocationStatement->argumentList.length);
        aliasedValue_t argumentList[tempLength];
        for (int32_t index = 0; index < tempLength; index++) {
            baseExpression_t *tempExpression;
            getVectorElement(&tempExpression, &(invocationStatement->argumentList), index);
            argumentList[index] = evaluateExpression(tempExpression);
        }
        invokeFunction(functionValue, argumentList, tempLength);
    }
    // TODO: Evaluate other types of statements.
    
}

void evaluateScript(script_t *script) {
    customFunctionHandle_t tempHandle;
    tempHandle.function = script->topLevelFunction;
    tempHandle.aliasList = NULL;
    script->globalFrame = invokeFunctionHandle(&tempHandle, NULL, 0);
}


