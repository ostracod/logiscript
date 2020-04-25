
#ifndef ERROR_HEADER_FILE
#define ERROR_HEADER_FILE

#include "value.h"
#include "parse.h"

#define ERROR_CHANNEL_CONSTANT 0

#define TYPE_ERROR_CONSTANT 0
#define NUMBER_ERROR_CONSTANT 1
#define DATA_ERROR_CONSTANT 2
#define STATE_ERROR_CONSTANT 3

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

int8_t hasThrownError;
int32_t thrownErrorChannel;
value_t thrownErrorValue;

void addErrorConstantsToNumberConstants(vector_t *destination);
void throwError(int32_t channel, value_t value);
void throwBuiltInError(int32_t errorCode, int8_t *message);
void addBodyPosToStackTrace(bodyPos_t *bodyPos);
void printStackTrace();

// ERROR_HEADER_FILE
#endif


