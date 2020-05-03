
#ifndef EXPRESSION_HEADER_FILE
#define EXPRESSION_HEADER_FILE

#include "vector.h"
#include "value.h"

#define EXPRESSION_TYPE_CONSTANT 1
#define EXPRESSION_TYPE_LIST 2
#define EXPRESSION_TYPE_FUNCTION 3
#define EXPRESSION_TYPE_IDENTIFIER 4
#define EXPRESSION_TYPE_VARIABLE 5
#define EXPRESSION_TYPE_UNARY 6
#define EXPRESSION_TYPE_BINARY 7
#define EXPRESSION_TYPE_INDEX 8
#define EXPRESSION_TYPE_INVOCATION 9

typedef struct script script_t;

typedef struct baseExpression {
    int8_t type;
    script_t *script;
} baseExpression_t;

typedef struct constantExpression {
    baseExpression_t base;
    value_t value;
} constantExpression_t;

typedef struct listExpression {
    baseExpression_t base;
    vector_t expressionList; // Vector of pointers to baseExpression_t.
} listExpression_t;

typedef struct customFunction customFunction_t;

typedef struct customFunctionExpression {
    baseExpression_t base;
    customFunction_t *customFunction;
} customFunctionExpression_t;

typedef struct identifierExpression {
    baseExpression_t base;
    int8_t *name;
} identifierExpression_t;

typedef struct baseScopeVariable baseScopeVariable_t;

typedef struct variableExpression {
    baseExpression_t base;
    baseScopeVariable_t *variable;
} variableExpression_t;

typedef struct operator operator_t;

typedef struct unaryExpression {
    baseExpression_t base;
    operator_t *operator;
    baseExpression_t *operand;
} unaryExpression_t;

typedef struct binaryExpression {
    baseExpression_t base;
    operator_t *operator;
    baseExpression_t *operand1;
    baseExpression_t *operand2;
} binaryExpression_t;

typedef struct indexExpression {
    baseExpression_t base;
    baseExpression_t *sequence;
    baseExpression_t *index;
} indexExpression_t;

typedef struct invocationExpression {
    baseExpression_t base;
    baseExpression_t *function;
    vector_t argumentList; // Vector of pointers to baseExpression_t.
} invocationExpression_t;

#include "script.h"
#include "variable.h"
#include "operator.h"
#include "function.h"

baseExpression_t *createIdentifierExpression(int8_t *name);
baseExpression_t *createConstantExpression(value_t value);
baseExpression_t *createListExpression(vector_t *expressionList);
baseExpression_t *createVariableExpression(baseScopeVariable_t *variable);
baseExpression_t *createUnaryExpression(operator_t *operator, baseExpression_t *operand);
baseExpression_t *createBinaryExpression(
    operator_t *operator,
    baseExpression_t *operand1,
    baseExpression_t *operand2
);
baseExpression_t *createCustomFunctionExpression(customFunction_t *customFunction);
baseExpression_t *createIndexExpression(
    baseExpression_t *sequenceExpression,
    baseExpression_t *indexExpression
);
baseExpression_t *createInvocationExpression(
    baseExpression_t *functionExpression,
    vector_t *expressionList
);

// EXPRESSION_HEADER_FILE
#endif


