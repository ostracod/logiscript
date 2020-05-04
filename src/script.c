
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
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
    free(script->moduleDirectory);
    free(script->path);
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

script_t *importScript(int8_t *moduleDirectory, int8_t *scriptPath) {
    
    // Verify script extension.
    int8_t *tempExtension = getFileExtension(scriptPath);
    if (tempExtension == NULL || strcmp((char *)tempExtension, "logi") != 0) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Script must have \".logi\" extension.");
        return NULL;
    }
    
    // Resolve script absolute path.
    int8_t *absolutePath;
    if (scriptPath[0] == '/') {
        absolutePath = mallocText(scriptPath);
    } else {
        int8_t *tempPath;
        // Note: POSIX treats double forward slash as a single slash.
        asprintf((char **)&tempPath, "%s/%s", moduleDirectory, scriptPath);
        absolutePath = mallocRealpath(tempPath);
        free(tempPath);
    }
    
    // Determine whether we have already imported the script.
    script_t *output = findScriptByPath(absolutePath);
    if (output != NULL) {
        free(absolutePath);
        return output;
    }
    
    // Create the script data structure.
    output = malloc(sizeof(script_t));
    output->body = NULL;
    output->moduleDirectory = mallocText(moduleDirectory);
    output->path = absolutePath;
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
    
    // Parse the script body.
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
    
    // Run the script.
    evaluateScript(output);
    if (hasThrownError) {
        return NULL;
    }
    pushVectorElement(&scriptList, &output);
    return output;
}

void importEntryPointScript(int8_t *path) {
    int8_t *tempPath = mallocRealpath(path);
    if (tempPath == NULL) {
        THROW_BUILT_IN_ERROR(
            STATE_ERROR_CONSTANT,
            "Could not read script at %s.",
            path
        );
        free(tempPath);
        return;
    }
    int8_t *tempPath2 = mallocText(tempPath);
    // dirname and basename do not malloc anything; instead, they
    // modify their input arguments.
    int8_t *moduleDirectory = (int8_t *)dirname((char *)tempPath);
    int8_t *scriptPath = (int8_t *)basename((char *)tempPath2);
    importScript(moduleDirectory, scriptPath);
    free(tempPath);
    free(tempPath2);
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


