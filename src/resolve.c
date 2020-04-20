
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "resolve.h"
#include "expression.h"
#include "statement.h"
#include "function.h"
#include "variable.h"

scopeVariable_t *resolveIdentifierInScope(scope_t *scope, int8_t *identifier) {
    scopeVariable_t *tempVariable = scopeFindVariable(scope, identifier);
    if (tempVariable != NULL) {
        return tempVariable;
    }
    if (scope->parentScope == NULL) {
        return NULL;
    }
    tempVariable = resolveIdentifierInScope(scope->parentScope, identifier);
    if (tempVariable == NULL) {
        return NULL;
    }
    return scopeAddVariable(scope, identifier, tempVariable->scopeIndex);
}

baseExpression_t *resolveIdentifierExpression(
    scope_t *scope,
    identifierExpression_t *identifierExpression
) {
    int8_t *tempName = identifierExpression->name;
    scopeVariable_t *tempVariable = resolveIdentifierInScope(scope, tempName);
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
    scope_t *scope,
    baseExpression_t **expression
) {
    baseExpression_t *tempExpression = *expression;
    switch (tempExpression->type) {
        case EXPRESSION_TYPE_IDENTIFIER:
        {
            baseExpression_t *tempResult = resolveIdentifierExpression(
                scope,
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
            resolveIdentifiersInExpression(scope, &(unaryExpression->operand));
            break;
        }
        case EXPRESSION_TYPE_BINARY:
        {
            binaryExpression_t *binaryExpression = (binaryExpression_t *)tempExpression;
            resolveIdentifiersInExpression(scope, &(binaryExpression->operand1));
            resolveIdentifiersInExpression(scope, &(binaryExpression->operand2));
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

void resolveIdentifiersInStatement(scope_t *scope, baseStatement_t *statement) {
    if (statement->type == STATEMENT_TYPE_INVOCATION) {
        invocationStatement_t *invocationStatement = (invocationStatement_t *)statement;
        resolveIdentifiersInExpression(scope, &(invocationStatement->function));
        for (int32_t index = 0; index < invocationStatement->argumentList.length; index++) {
            baseExpression_t **tempExpression = findVectorElement(
                &(invocationStatement->argumentList),
                index
            );
            resolveIdentifiersInExpression(scope, tempExpression);
        }
    }
    // TODO: Resolve identifiers in other statement types.
    
}

void resolveIdentifiersInFunction(customFunction_t *function) {
    for (int64_t index = 0; index < function->statementList.length; index++) {
        baseStatement_t *tempStatement;
        getVectorElement(&tempStatement, &(function->statementList), index);
        resolveIdentifiersInStatement(&(function->scope), tempStatement);
    }
}


