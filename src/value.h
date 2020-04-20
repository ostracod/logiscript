
#ifndef VALUE_HEADER_FILE
#define VALUE_HEADER_FILE

#include "vector.h"

#define VALUE_TYPE_ALIAS 1
#define VALUE_TYPE_VOID 2
#define VALUE_TYPE_NUMBER 3
#define VALUE_TYPE_STRING 4
#define VALUE_TYPE_LIST 5
#define VALUE_TYPE_FRAME 6
#define VALUE_TYPE_BUILT_IN_FUNCTION 7
#define VALUE_TYPE_CUSTOM_FUNCTION 8

typedef struct heapValue heapValue_t;
typedef struct builtInFunction builtInFunction_t;

typedef struct alias {
    heapValue_t *container;
    int64_t index;
} alias_t;

typedef struct value {
    int8_t type;
    // For void, the union is unused.
    // For numbers, the union is a double.
    // For strings, lists, frames, and custom functions, the union is a
    // pointer to heapValue_t.
    // For built-in functions, the union is a pointer to builtInFunction_t.
    // For aliases, the union is an alias_t.
    union {
        double numberValue;
        heapValue_t *heapValue;
        builtInFunction_t *builtInFunction;
        alias_t alias;
    };
} value_t;

typedef struct customFunctionHandle customFunctionHandle_t;

typedef struct heapValue {
    int8_t type;
    int8_t isMarked;
    int32_t lockDepth;
    int64_t referenceCount;
    heapValue_t *previous;
    heapValue_t *next;
    // For strings, the union is a vector of int8_t.
    // For lists, the union is a vector of value_t.
    // For frames, the union is an array of value_t.
    // For custom functions, the union is a customFunctionHandle_t.
    union {
        vector_t vector;
        value_t *frameVariableList;
        customFunctionHandle_t *customFunctionHandle;
    };
} heapValue_t;

heapValue_t *createHeapValue(int8_t type);
value_t createValueFromHeapValue(heapValue_t *heapValue);
value_t copyValue(value_t value);
value_t convertValueToString(value_t value, int8_t shouldCopy);
value_t readValueFromAlias(alias_t alias);
void writeValueToAlias(alias_t alias, value_t value);
value_t resolveAliasValue(value_t value);
void writeValueToAliasValue(value_t aliasValue, value_t value);

#include "function.h"
#include "variable.h"

// VALUE_HEADER_FILE
#endif


