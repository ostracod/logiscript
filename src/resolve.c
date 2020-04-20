
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "resolve.h"
#include "expression.h"
#include "statement.h"
#include "function.h"
#include "variable.h"

baseExpression_t *resolveIdentifierExpression(
    customFunction_t *function,
    identifierExpression_t *identifierExpression
) {
    int8_t *tempName = identifierExpression->name;
    scopeVariable_t *tempVariable = scopeFindVariable(&(function->scope), tempName);
    if (tempVariable != NULL) {
        return createVariableExpression(tempVariable);
    }
    builtInFunction_t *tempFunction = findBuiltInFunctionByName(tempName);
    if (tempFunction != NULL) {
        value_t tempValue;
        tempValue.type = VALUE_TYPE_BUILT_IN_FUNCTION;
        tempValue.builtInFunction = tempFunction;
        return createConstantExpression(tempValue);
    }
    // TODO: Resolve other types of identifiers.
    
    return NULL;
}

void resolveIdentifiersInExpression(
    customFunction_t *function,
    baseExpression_t **expression
) {
    baseExpression_t *tempExpression = *expression;
    switch (tempExpression->type) {
        case EXPRESSION_TYPE_IDENTIFIER:
        {
            baseExpression_t *tempResult = resolveIdentifierExpression(
                function,
                (identifierExpression_t *)tempExpression
            );
            if (tempResult != NULL) {
                *expression = tempResult;
            }
            break;
        }
        case EXPRESSION_TYPE_UNARY:
        {
            unaryExpression_t *unaryExpression = (unaryExpression_t *)tempExpression;
            resolveIdentifiersInExpression(function, &(unaryExpression->operand));
            break;
        }
        case EXPRESSION_TYPE_BINARY:
        {
            binaryExpression_t *binaryExpression = (binaryExpression_t *)tempExpression;
            resolveIdentifiersInExpression(function, &(binaryExpression->operand1));
            resolveIdentifiersInExpression(function, &(binaryExpression->operand2));
            break;
        }
        case EXPRESSION_TYPE_FUNCTION:
        {
            resolveIdentifiersInFunction(
                ((customFunctionExpression_t *)tempExpression)->customFunction
            );
            break;
        }
        // TODO: Resolve identifiers in other expression types.
        
        default:
        {
            break;
        }
    }
}

void resolveIdentifiersInStatement(customFunction_t *function, baseStatement_t *statement) {
    if (statement->type == STATEMENT_TYPE_INVOCATION) {
        invocationStatement_t *invocationStatement = (invocationStatement_t *)statement;
        resolveIdentifiersInExpression(function, &(invocationStatement->function));
        for (int32_t index = 0; index < invocationStatement->argumentList.length; index++) {
            baseExpression_t **tempExpression = findVectorElement(
                &(invocationStatement->argumentList),
                index
            );
            resolveIdentifiersInExpression(function, tempExpression);
        }
    }
    // TODO: Resolve identifiers in other statement types.
    
}

void resolveIdentifiersInFunction(customFunction_t *function) {
    for (int64_t index = 0; index < function->statementList.length; index++) {
        baseStatement_t *tempStatement;
        getVectorElement(&tempStatement, &(function->statementList), index);
        resolveIdentifiersInStatement(function, tempStatement);
    }
}


