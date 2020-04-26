
#ifndef VALUE_HEADER_FILE
#define VALUE_HEADER_FILE

#include "vector.h"

#define VALUE_TYPE_VOID 1
#define VALUE_TYPE_NUMBER 2
#define VALUE_TYPE_STRING 3
#define VALUE_TYPE_LIST 4
#define VALUE_TYPE_FRAME 5
#define VALUE_TYPE_BUILT_IN_FUNCTION 6
#define VALUE_TYPE_CUSTOM_FUNCTION 7

#define HYPER_VALUE_TYPE_VALUE 1
#define HYPER_VALUE_TYPE_ALIAS 2

typedef struct heapValue heapValue_t;
typedef struct builtInFunction builtInFunction_t;

typedef struct value {
    int8_t type;
    // For void, the union is unused.
    // For numbers, the union is a double.
    // For strings, lists, frames, and custom functions, the union is a
    // pointer to heapValue_t.
    // For built-in functions, the union is a pointer to builtInFunction_t.
    union {
        double numberValue;
        heapValue_t *heapValue;
        builtInFunction_t *builtInFunction;
    };
} value_t;

// Note: Aliases cannot reference other aliases.
typedef struct alias {
    heapValue_t *container;
    int64_t index;
} alias_t;

typedef struct hyperValue {
    int8_t type;
    union {
        value_t value;
        alias_t alias;
    };
} hyperValue_t;

typedef struct hyperValueList {
    hyperValue_t *valueArray;
    int32_t length;
} hyperValueList_t;

typedef struct customFunctionHandle customFunctionHandle_t;

typedef struct heapValue {
    int8_t type;
    int8_t isMarked;
    int32_t lockDepth;
    int64_t referenceCount;
    int64_t aliasCount;
    heapValue_t *previous;
    heapValue_t *next;
    // For strings, the union is a vector of int8_t.
    // For lists, the union is a vector of value_t.
    // For frames, the union is a hyperValueList_t.
    // For custom functions, the union is a pointer to customFunctionHandle_t.
    union {
        vector_t vector;
        hyperValueList_t frameVariableList;
        customFunctionHandle_t *customFunctionHandle;
    };
} heapValue_t;

heapValue_t *createHeapValue(int8_t type);
value_t createValueFromHeapValue(heapValue_t *heapValue);
value_t copyValue(value_t value);
value_t convertTextToStringValue(int8_t *text);
value_t convertValueToString(value_t value, int8_t shouldCopy);
value_t readValueFromAlias(alias_t alias);
void writeValueToAlias(alias_t alias, value_t value);
value_t resolveAliasValue(hyperValue_t hyperValue);
void writeValueToAliasValue(hyperValue_t aliasValue, value_t value);

#include "function.h"
#include "variable.h"

// VALUE_HEADER_FILE
#endif


