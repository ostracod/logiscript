
#ifndef STATEMENT_HEADER_FILE
#define STATEMENT_HEADER_FILE

#include "vector.h"
#include "script.h"
#include "parse.h"
#include "expression.h"

#define STATEMENT_TYPE_INVOCATION 1
#define STATEMENT_TYPE_NAMESPACE_IMPORT 2
#define STATEMENT_TYPE_VARIABLE_IMPORT 3

typedef struct baseStatement {
    int8_t type;
    bodyPos_t bodyPos;
} baseStatement_t;

typedef struct invocationStatement {
    baseStatement_t base;
    baseExpression_t *function;
    vector_t argumentList; // Vector of pointers to baseExpression_t.
} invocationStatement_t;

typedef struct baseImportStatement {
    baseStatement_t base;
    baseExpression_t *path;
} baseImportStatement_t;

typedef struct namespaceImportStatement {
    baseImportStatement_t base;
    namespace_t *namespace;
} namespaceImportStatement_t;

typedef struct variableImportStatement {
    baseImportStatement_t base;
    vector_t variableList; // Vector of pointers to baseScopeVariable_t.
} variableImportStatement_t;

// STATEMENT_HEADER_FILE
#endif


