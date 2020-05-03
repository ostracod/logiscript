
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "function.h"
#include "variable.h"
#include "error.h"

int8_t scopeVariableTypeAllowsDuplicates(int8_t type) {
    return (type != SCOPE_VARIABLE_TYPE_ARGUMENT && type != SCOPE_VARIABLE_TYPE_IMPORT);
}

baseScopeVariable_t *scopeAddVariableHelper(
    scope_t *scope,
    int8_t *name,
    namespace_t *namespace,
    int8_t type,
    int32_t size
) {
    baseScopeVariable_t *tempVariable = scopeFindVariable(scope, name, namespace);
    if (tempVariable != NULL) {
        if (!scopeVariableTypeAllowsDuplicates(tempVariable->type)
                || !scopeVariableTypeAllowsDuplicates(type)) {
            THROW_BUILT_IN_ERROR(
                PARSE_ERROR_CONSTANT,
                "Duplicate variable \"%s\".",
                name
            );
            return NULL;
        }
        return tempVariable;
    }
    baseScopeVariable_t *output = malloc(size);
    output->type = type;
    output->name = name;
    output->scopeIndex = scope->variableList.length;
    pushVectorElement(&(scope->variableList), &output);
    return output;
}

baseScopeVariable_t *scopeAddBaseVariable(scope_t *scope, int8_t *name, int8_t type) {
    return scopeAddVariableHelper(scope, name, NULL, type, sizeof(baseScopeVariable_t));
}

baseScopeVariable_t *scopeAddArgumentVariable(scope_t *scope, int8_t *name) {
    return scopeAddBaseVariable(scope, name, SCOPE_VARIABLE_TYPE_ARGUMENT);
}

baseScopeVariable_t *scopeAddLocalVariable(scope_t *scope, int8_t *name) {
    return scopeAddBaseVariable(scope, name, SCOPE_VARIABLE_TYPE_LOCAL);
}

baseScopeVariable_t *scopeAddParentVariable(
    scope_t *scope,
    int8_t *name,
    int32_t parentScopeIndex
) {
    parentScopeVariable_t *output = (parentScopeVariable_t *)scopeAddVariableHelper(
        scope,
        name,
        NULL,
        SCOPE_VARIABLE_TYPE_PARENT,
        sizeof(parentScopeVariable_t)
    );
    if (hasThrownError) {
        return NULL;
    }
    output->parentScopeIndex = parentScopeIndex;
    scope->parentVariableAmount += 1;
    return (baseScopeVariable_t *)output;
}

baseScopeVariable_t *scopeAddImportVariable(scope_t *scope, int8_t *name) {
    return scopeAddBaseVariable(scope, name, SCOPE_VARIABLE_TYPE_IMPORT);
}

baseScopeVariable_t *scopeAddNamespaceVariable(
    scope_t *scope,
    int8_t *name,
    namespace_t *namespace
) {
    namespaceScopeVariable_t *output = (namespaceScopeVariable_t *)scopeAddVariableHelper(
        scope,
        name,
        namespace,
        SCOPE_VARIABLE_TYPE_NAMESPACE,
        sizeof(namespaceScopeVariable_t)
    );
    if (hasThrownError) {
        return NULL;
    }
    output->namespace = namespace;
    if (namespace != NULL) {
        pushVectorElement(&(namespace->variableList), &output);
    }
    return (baseScopeVariable_t *)output;
}

namespace_t *getScopeVariableNamespace(baseScopeVariable_t *variable) {
    if (variable->type == SCOPE_VARIABLE_TYPE_NAMESPACE) {
        namespaceScopeVariable_t *namespaceScopeVariable = (namespaceScopeVariable_t *)variable;
        return namespaceScopeVariable->namespace;
    }
    return NULL;
}

baseScopeVariable_t *scopeFindVariable(scope_t *scope, int8_t *name, namespace_t *namespace) {
    for (int32_t index = 0; index < scope->variableList.length; index++) {
        baseScopeVariable_t *tempVariable;
        getVectorElement(&tempVariable, &(scope->variableList), index);
        if (strcmp((char *)name, (char *)(tempVariable->name)) == 0
                && getScopeVariableNamespace(tempVariable) == namespace) {
            return tempVariable;
        }
    }
    return NULL;
}

hyperValue_t getFrameVariableLocation(heapValue_t *frame, int32_t index) {
    hyperValue_t tempValue = frame->frameVariableList.valueArray[index];
    if (tempValue.type == HYPER_VALUE_TYPE_VALUE) {
        hyperValue_t output;
        output.type = HYPER_VALUE_TYPE_ALIAS;
        output.alias.container = frame;
        output.alias.index = index;
        return output;
    } else if (tempValue.type == HYPER_VALUE_TYPE_ALIAS
            && tempValue.alias.container == NULL) {
        THROW_BUILT_IN_ERROR(STATE_ERROR_CONSTANT, "Variable has not yet been imported.");
        hyperValue_t output;
        output.type = HYPER_VALUE_TYPE_VALUE;
        output.value.type = VALUE_TYPE_VOID;
        return output;
    }
    return tempValue;
}


