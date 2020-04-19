
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "resolve.h"
#include "expression.h"
#include "statement.h"
#include "function.h"

baseExpression_t *resolveIdentifierExpression(identifierExpression_t *identifierExpression) {
    int8_t *tempName = identifierExpression->name;
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

void resolveIdentifiersInExpression(baseExpression_t **expression) {
    baseExpression_t *tempExpression = *expression;
    switch (tempExpression->type) {
        case EXPRESSION_TYPE_IDENTIFIER:
        {
            baseExpression_t *tempResult = resolveIdentifierExpression(
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
            resolveIdentifiersInExpression(&(unaryExpression->operand));
            break;
        }
        case EXPRESSION_TYPE_BINARY:
        {
            binaryExpression_t *binaryExpression = (binaryExpression_t *)tempExpression;
            resolveIdentifiersInExpression(&(binaryExpression->operand1));
            resolveIdentifiersInExpression(&(binaryExpression->operand2));
            break;
        }
        // TODO: Resolve identifiers in other expression types.
        
        default:
        {
            break;
        }
    }
}

void resolveIdentifiersInStatement(baseStatement_t *statement) {
    if (statement->type == STATEMENT_TYPE_INVOCATION) {
        invocationStatement_t *invocationStatement = (invocationStatement_t *)statement;
        resolveIdentifiersInExpression(&(invocationStatement->function));
        for (int32_t index = 0; index < invocationStatement->argumentList.length; index++) {
            baseExpression_t **tempExpression = findVectorElement(
                &(invocationStatement->argumentList),
                index
            );
            resolveIdentifiersInExpression(tempExpression);
        }
    }
    // TODO: Resolve identifiers in other statement types.
    
}

void resolveIdentifiersInFunction(customFunction_t *function) {
    for (int64_t index = 0; index < function->statementList.length; index++) {
        baseStatement_t *tempStatement;
        getVectorElement(&tempStatement, &(function->statementList), index);
        resolveIdentifiersInStatement(tempStatement);
    }
}


