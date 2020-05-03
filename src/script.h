
#ifndef SCRIPT_HEADER_FILE
#define SCRIPT_HEADER_FILE

#include "parse.h"
#include "function.h"
#include "value.h"
#include "variable.h"

typedef struct script {
    int8_t *path;
    int8_t *body;
    int64_t bodyLength;
    customFunction_t *topLevelFunction;
    heapValue_t *globalFrame;
    vector_t namespaceList; // Vector of pointers to namespace_t.
} script_t;

script_t *importScript(int8_t *path);
namespace_t *scriptFindNamespace(script_t *script, int8_t *name);

// SCRIPT_HEADER_FILE
#endif


