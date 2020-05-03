
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
#include "resolve.h"
#include "evaluate.h"
#include "error.h"

void cleanUpScript(script_t *script) {
    if (script->path != NULL) {
        free(script->path);
    }
    if (script->body != NULL) {
        free(script->body);
    }
    free(script);
}

script_t *findScriptByPath(int8_t *path) {
    for (int32_t index = 0; index < scriptList.length; index++) {
        script_t *tempScript;
        getVectorElement(&tempScript, &scriptList, index);
        if (strcmp((char *)path, (char *)(tempScript->path)) == 0) {
            return tempScript;
        }
    }
    return NULL;
}

script_t *importScript(int8_t *path) {
    int8_t *tempPath = mallocRealpath(path);
    script_t *output = findScriptByPath(tempPath);
    if (output != NULL) {
        free(tempPath);
        return output;
    }
    output = malloc(sizeof(script_t));
    output->body = NULL;
    output->path = tempPath;
    if (output->path == NULL) {
        THROW_BUILT_IN_ERROR(
            STATE_ERROR_CONSTANT,
            "Could not read script at %s.",
            path
        );
        cleanUpScript(output);
        return NULL;
    }
    output->body = readEntireFile(&(output->bodyLength), output->path);
    if (output->body == NULL) {
        THROW_BUILT_IN_ERROR(
            STATE_ERROR_CONSTANT,
            "Could not read script at %s.",
            output->path
        );
        cleanUpScript(output);
        return NULL;
    }
    
    customFunction_t *topLevelFunction = createEmptyCustomFunction(output, NULL);
    output->topLevelFunction = topLevelFunction;
    createEmptyVector(&(output->namespaceList), sizeof(namespace_t *));
    
    bodyPos_t bodyPos;
    bodyPos.script = output;
    bodyPos.index = 0;
    bodyPos.lineNumber = 1;
    parser_t parser;
    parser.script = output;
    parser.customFunction = topLevelFunction;
    parser.bodyPos = &bodyPos;
    parseStatementList(&(topLevelFunction->statementList), &parser, -1);
    if (hasThrownError) {
        addBodyPosToStackTrace(parser.bodyPos);
        cleanUpScript(output);
        return NULL;
    }
    resolveIdentifiersInFunction(topLevelFunction);
    if (hasThrownError) {
        cleanUpScript(output);
        return NULL;
    }
    evaluateScript(output);
    if (hasThrownError) {
        return NULL;
    }
    pushVectorElement(&scriptList, &output);
    return output;
}

namespace_t *scriptFindNamespace(script_t *script, int8_t *name) {
    for (int32_t index = 0; index < script->namespaceList.length; index++) {
        namespace_t *tempNamespace;
        getVectorElement(&tempNamespace, &(script->namespaceList), index);
        if (strcmp((char *)name, (char *)(tempNamespace->name)) == 0) {
            return tempNamespace;
        }
    }
    return NULL;
}


