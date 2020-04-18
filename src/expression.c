
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "expression.h"

baseExpression_t *createIdentifierExpression(int8_t *name, int64_t length) {
    identifierExpression_t *output = malloc(sizeof(identifierExpression_t));
    output->base.type = EXPRESSION_TYPE_IDENTIFIER;
    output->name = mallocText(name, length);
    return (baseExpression_t *)output;
}

baseExpression_t *createConstantExpression(value_t value) {
    constantExpression_t *output = malloc(sizeof(constantExpression_t));
    output->base.type = EXPRESSION_TYPE_CONSTANT;
    output->value = value;
    return (baseExpression_t *)output;
}

baseExpression_t *createUnaryExpression(operator_t *operator, baseExpression_t *operand) {
    unaryExpression_t *output = malloc(sizeof(unaryExpression_t));
    output->base.type = EXPRESSION_TYPE_UNARY;
    output->operator = operator;
    output->operand = operand;
    return (baseExpression_t *)output;
}


