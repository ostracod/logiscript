
#ifndef FUNCTION_HEADER_FILE
#define FUNCTION_HEADER_FILE

#include "vector.h"
#include "variable.h"
#include "value.h"

#define FUNCTION_TYPE_BUILT_IN 1
#define FUNCTION_TYPE_CUSTOM 2

#define BUILT_IN_FUNCTION_SET 1
#define BUILT_IN_FUNCTION_TYPE 2
#define BUILT_IN_FUNCTION_CONVERT 3
#define BUILT_IN_FUNCTION_SIZE 4
#define BUILT_IN_FUNCTION_RESIZE 5
#define BUILT_IN_FUNCTION_IF 6
#define BUILT_IN_FUNCTION_LOOP 7
#define BUILT_IN_FUNCTION_THROW 8
#define BUILT_IN_FUNCTION_CATCH 9
#define BUILT_IN_FUNCTION_PRINT 10
#define BUILT_IN_FUNCTION_PROMPT 11

typedef struct baseFunction {
    int8_t type;
    int32_t argumentAmount;
} baseFunction_t;

typedef struct builtInFunction {
    baseFunction_t base;
    int8_t *name;
    int32_t number;
} builtInFunction_t;

typedef struct customFunction {
    baseFunction_t base;
    vector_t statementList; // Vector of pointers to baseStatement_t.
    scope_t scope;
} customFunction_t;

typedef struct customFunctionHandle {
    customFunction_t *function;
    hyperValueList_t locationList;
} customFunctionHandle_t;

builtInFunction_t *findBuiltInFunctionByName(int8_t *name);
heapValue_t *createFunctionHandle(heapValue_t *frame, customFunction_t *customFunction);
heapValue_t *invokeFunctionHandle(
    heapValue_t *functionHandle,
    hyperValueList_t *argumentList
);
void invokeFunction(value_t functionValue, hyperValueList_t *argumentList);

// FUNCTION_HEADER_FILE
#endif


