
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "error.h"

void throwError(int32_t channel, value_t value) {
    thrownErrorChannel = channel;
    thrownErrorValue = value;
    hasThrownError = true;
}


