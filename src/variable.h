
#ifndef VARIABLE_HEADER_FILE
#define VARIABLE_HEADER_FILE

#include "vector.h"
#include "value.h"

#define FRAME_VARIABLE_TYPE_VALUE 1
#define FRAME_VARIABLE_TYPE_ALIAS 2

typedef struct scopeVariable {
    int8_t *name;
    int32_t scopeIndex;
    // parentScopeIndex will be -1 if the scope variable does
    // not alias a variable in the parent scope.
    int32_t parentScopeIndex;
} scopeVariable_t;

typedef struct frameVariable {
    int8_t type;
    union {
        value_t value;
        alias_t alias;
    };
} frameVariable_t;

typedef struct scope {
    int32_t aliasVariableAmount;
    // Function argument variables will preceed any other
    // variables in the scope. Argument variables will be
    // populated in the same order as provided by invocation.
    vector_t variableList; // Vector of scopeVariable_t.
} scope_t;

// VARIABLE_HEADER_FILE
#endif


