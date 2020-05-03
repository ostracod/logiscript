
#ifndef VARIABLE_HEADER_FILE
#define VARIABLE_HEADER_FILE

#define SCOPE_VARIABLE_TYPE_ARGUMENT 1
#define SCOPE_VARIABLE_TYPE_LOCAL 2
#define SCOPE_VARIABLE_TYPE_PARENT 3
#define SCOPE_VARIABLE_TYPE_IMPORT 4
#define SCOPE_VARIABLE_TYPE_NAMESPACE 5

typedef struct baseScopeVariable {
    int8_t type;
    int8_t *name;
    int32_t scopeIndex;
} baseScopeVariable_t;

typedef struct parentScopeVariable {
    baseScopeVariable_t base;
    int32_t parentScopeIndex;
} parentScopeVariable_t;

typedef struct namespace namespace_t;

typedef struct namespaceScopeVariable {
    baseScopeVariable_t base;
    namespace_t *namespace;
} namespaceScopeVariable_t;

typedef struct scope scope_t;
typedef struct script script_t;

typedef struct scope {
    script_t *script;
    scope_t *parentScope;
    int32_t parentVariableAmount;
    // Function argument variables will preceed any other
    // variables in the scope. Argument variables will be
    // populated in the same order as provided by invocation.
    vector_t variableList; // Vector of pointers to baseScopeVariable_t.
} scope_t;

typedef struct namespace {
    int8_t *name;
    vector_t variableList; // Vector of pointers to namespaceScopeVariable_t.
} namespace_t;

#include "vector.h"
#include "value.h"
#include "script.h"

baseScopeVariable_t *scopeAddArgumentVariable(scope_t *scope, int8_t *name);
baseScopeVariable_t *scopeAddLocalVariable(scope_t *scope, int8_t *name);
baseScopeVariable_t *scopeAddParentVariable(
    scope_t *scope,
    int8_t *name,
    int32_t parentScopeIndex
);
baseScopeVariable_t *scopeAddImportVariable(scope_t *scope, int8_t *name);
baseScopeVariable_t *scopeAddNamespaceVariable(
    scope_t *scope,
    int8_t *name,
    namespace_t *namespace
);
baseScopeVariable_t *scopeFindVariable(scope_t *scope, int8_t *name, namespace_t *namespace);
hyperValue_t getFrameVariableLocation(heapValue_t *frame, int32_t index);

// VARIABLE_HEADER_FILE
#endif


