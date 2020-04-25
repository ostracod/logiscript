
#ifndef VARIABLE_HEADER_FILE
#define VARIABLE_HEADER_FILE

#include "vector.h"
#include "value.h"

typedef struct scopeVariable {
    int8_t *name;
    int32_t scopeIndex;
    // parentScopeIndex will be -1 if the scope variable does
    // not alias a variable in the parent scope.
    int32_t parentScopeIndex;
} scopeVariable_t;

typedef struct scope scope_t;

typedef struct scope {
    scope_t *parentScope;
    int32_t aliasVariableAmount;
    // Function argument variables will preceed any other
    // variables in the scope. Argument variables will be
    // populated in the same order as provided by invocation.
    vector_t variableList; // Vector of pointers to scopeVariable_t.
} scope_t;

scopeVariable_t *scopeAddVariable(
    scope_t *scope,
    int8_t *name,
    int32_t parentScopeIndex,
    int8_t allowDuplicates
);
scopeVariable_t *scopeFindVariable(scope_t *scope, int8_t *name);
alias_t getAliasToFrameVariable(heapValue_t *frame, int32_t index);

// VARIABLE_HEADER_FILE
#endif


