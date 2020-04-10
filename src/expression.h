
#ifndef EXPRESSION_HEADER_FILE
#define EXPRESSION_HEADER_FILE

#include "vector.h"
#include "script.h"
#include "value.h"
#include "variable.h"
#include "operator.h"
#include "function.h"

#define EXPRESSION_TYPE_CONSTANT 1
#define EXPRESSION_TYPE_LIST 2
#define EXPRESSION_TYPE_FUNCTION 3
#define EXPRESSION_TYPE_IDENTIFIER 4
#define EXPRESSION_TYPE_VARIABLE 5
#define EXPRESSION_TYPE_NAMESPACE 6
#define EXPRESSION_TYPE_UNARY 7
#define EXPRESSION_TYPE_BINARY 8
#define EXPRESSION_TYPE_INDEX 9
#define EXPRESSION_TYPE_INVOCATION 10

typedef struct baseExpression {
    int8_t type;
} baseExpression_t;

typedef struct constantExpression {
    baseExpression_t base;
    value_t value;
} constantExpression_t;

typedef struct listExpression {
    baseExpression_t base;
    vector_t expressionList; // Vector of pointers to baseExpression_t.
} listExpression_t;

typedef struct customFunctionExpression {
    baseExpression_t base;
    customFunction_t *customFunction;
} customFunctionExpression_t;

typedef struct identifierExpression {
    baseExpression_t base;
    int8_t *name;
} identifierExpression_t;

typedef struct variableExpression {
    baseExpression_t base;
    scopeVariable_t *variable;
} variableExpression_t;

typedef struct namespaceExpression {
    baseExpression_t base;
    namespace_t *namespace;
} namespaceExpression_t;

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
    baseExpression_t *list;
    baseExpression_t *index;
} indexExpression_t;

typedef struct invocationExpression {
    baseExpression_t base;
    baseExpression_t *function;
    vector_t argumentList; // Vector of pointers to baseExpression_t.
} invocationExpression_t;

// EXPRESSION_HEADER_FILE
#endif


