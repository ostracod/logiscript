
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
#define HYPER_VALUE_TYPE_POINTER 3

#define HEAP_VALUE_MARK_NONE 1
#define HEAP_VALUE_MARK_WEAK 2
#define HEAP_VALUE_MARK_STRONG 3

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
        value_t *valuePointer;
    };
} hyperValue_t;

typedef struct hyperValueList {
    // valueArray may be NULL if length is 0.
    hyperValue_t *valueArray;
    int32_t length;
} hyperValueList_t;

typedef struct customFunctionHandle customFunctionHandle_t;

typedef struct heapValue {
    int8_t type;
    int8_t mark;
    int32_t lockDepth;
    int64_t referenceCount;
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
void deleteValueIfUnreferenced(value_t *value);
void lockHeapValue(heapValue_t *heapValue);
void lockValue(value_t *value);
void lockHyperValue(hyperValue_t *hyperValue);
void unlockHeapValue(heapValue_t *heapValue);
void unlockValue(value_t *value);
void unlockHyperValue(hyperValue_t *hyperValue);
void unlockFunctionInvocationValues(
    value_t *function,
    hyperValueList_t *hyperValueList,
    int8_t hasDestinationArgument
);
void unlockValuesInVector(vector_t *vector);
void addHyperValueReference(hyperValue_t *hyperValue);
void addValueReferencesInVector(vector_t *vector);
void removeValueReference(value_t *value);
void swapHyperValueReference(hyperValue_t *destination, hyperValue_t *source);
void markAndSweepHeapValues();
value_t createValueFromHeapValue(heapValue_t *heapValue);
value_t copyValue(value_t value);
value_t convertTextToStringValue(int8_t *text);
value_t convertValueToString(value_t value, int8_t shouldCopy);
value_t readValueFromAlias(alias_t alias, int8_t shouldThrowError);
void writeValueToAlias(alias_t alias, value_t value);
value_t readValueFromHyperValue(hyperValue_t hyperValue);
void writeValueToLocation(hyperValue_t location, value_t value);
void printAllHeapValues();

#include "function.h"
#include "variable.h"

// VALUE_HEADER_FILE
#endif


