
#ifndef SCRIPT_HEADER_FILE
#define SCRIPT_HEADER_FILE

#include "vector.h"

typedef struct customFunction customFunction_t;
typedef struct heapValue heapValue_t;

typedef struct script {
    int8_t *moduleDirectory;
    int8_t *path;
    int8_t *body;
    int64_t bodyLength;
    customFunction_t *topLevelFunction;
    heapValue_t *globalFrame;
    vector_t namespaceList; // Vector of pointers to namespace_t.
} script_t;

#include "function.h"
#include "value.h"

vector_t scriptList; // Vector of pointers to script_t.

script_t *importScript(int8_t *moduleDirectory, int8_t *scriptPath);
void importEntryPointScript(int8_t *path);
namespace_t *scriptFindNamespace(script_t *script, int8_t *name);

// SCRIPT_HEADER_FILE
#endif


