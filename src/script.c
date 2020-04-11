
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "vector.h"
#include "script.h"
#include "function.h"
#include "variable.h"
#include "parse.h"

script_t *importScript(int8_t *path) {
    script_t *output = malloc(sizeof(script_t));
    output->path = mallocRealpath(path);
    output->body = readEntireFile(&(output->bodyLength), output->path);
    if (output->body == NULL) {
        free(output);
        return NULL;
    }
    
    customFunction_t *topLevelFunction = malloc(sizeof(customFunction_t));
    topLevelFunction->base.type = FUNCTION_TYPE_CUSTOM;
    topLevelFunction->base.argumentAmount = 0;
    scope_t *scope = &(topLevelFunction->scope);
    scope->aliasVariableAmount = 0;
    createEmptyVector(&(scope->variableList), sizeof(scopeVariable_t));
    output->topLevelFunction = topLevelFunction;
    // TODO: Create global frame of output.
    createEmptyVector(&(output->namespaceList), sizeof(namespace_t *));
    
    bodyPos_t bodyPos;
    bodyPos.script = output;
    bodyPos.index = 0;
    bodyPos.lineNumber = 1;
    parser_t parser;
    parser.script = output;
    parser.customFunction = topLevelFunction;
    parser.bodyPos = &bodyPos;
    parseStatementList(&(topLevelFunction->statementList), &parser);
    return output;
}


