
#ifndef PARSE_HEADER_FILE
#define PARSE_HEADER_FILE

#include "script.h"

typedef struct script script_t;

typedef struct bodyLine {
    script_t *script;
    int64_t index;
    int64_t number;
} bodyLine_t;

// PARSE_HEADER_FILE
#endif


