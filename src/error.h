
#ifndef ERROR_HEADER_FILE
#define ERROR_HEADER_FILE

#define ERROR_CHANNEL_CONSTANT 0

#define PARSE_ERROR_CONSTANT 0
#define TYPE_ERROR_CONSTANT 1
#define NUMBER_ERROR_CONSTANT 2
#define DATA_ERROR_CONSTANT 3
#define STATE_ERROR_CONSTANT 4

#define MAXIMUM_STACK_TRACE_LENGTH 10

#define THROW_BUILT_IN_ERROR(errorCode, ...) {\
    int8_t *errorText;\
    asprintf((char **)&errorText, __VA_ARGS__);\
    throwBuiltInError(errorCode, errorText);\
    free(errorText);\
}

typedef struct errorConstant {
    int8_t *name;
    int32_t code;
} errorConstant_t;

typedef struct stackTracePos {
    int64_t lineNumber;
    int8_t *path;
    int8_t hasCopiedPath;
} stackTracePos_t;

#include "value.h"
#include "parse.h"

int8_t hasThrownError;
int32_t thrownErrorChannel;
value_t thrownErrorValue;

void addErrorConstantsToNumberConstants(vector_t *destination);
void throwError(int32_t channel, value_t value);
void throwBuiltInError(int32_t errorCode, int8_t *message);
void addBodyPosToStackTrace(bodyPos_t *bodyPos);
void copyStackTracePosPaths();
void printStackTrace();
int32_t getStackTraceLength();

// ERROR_HEADER_FILE
#endif


