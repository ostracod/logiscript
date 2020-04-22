
#ifndef ERROR_HEADER_FILE
#define ERROR_HEADER_FILE

#include "value.h"

#define ERROR_CHANNEL_CONSTANT 0

#define TYPE_ERROR_CONSTANT 0
#define NUMBER_ERROR_CONSTANT 1
#define DATA_ERROR_CONSTANT 2
#define MISSING_ERROR_CONSTANT 3

int8_t hasThrownError;
int32_t thrownErrorChannel;
value_t thrownErrorValue;

void throwError(int32_t channel, value_t value);
void throwBuiltInError(int32_t errorCode, int8_t *message);

// ERROR_HEADER_FILE
#endif


