
#ifndef RESOLVE_HEADER_FILE
#define RESOLVE_HEADER_FILE

#define NUMBER_TYPE_CONSTANT 0
#define STRING_TYPE_CONSTANT 1
#define LIST_TYPE_CONSTANT 2
#define FUNCTION_TYPE_CONSTANT 3
#define VOID_TYPE_CONSTANT 4

typedef struct numberConstant {
    int8_t *name;
    double value;
} numberConstant_t;

#include "function.h"

void initializeNumberConstants();
void resolveIdentifiersInFunction(customFunction_t *function);

// RESOLVE_HEADER_FILE
#endif


