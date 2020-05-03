
#ifndef PARSE_HEADER_FILE
#define PARSE_HEADER_FILE


typedef struct script script_t;

typedef struct bodyPos {
    script_t *script;
    int64_t index;
    int64_t lineNumber;
} bodyPos_t;

typedef struct customFunction customFunction_t;

typedef struct parser {
    script_t *script;
    customFunction_t *customFunction;
    bodyPos_t *bodyPos;
} parser_t;

#include "script.h"
#include "function.h"

void parseStatementList(vector_t *destination, parser_t *parser, int8_t endCharacter);

// PARSE_HEADER_FILE
#endif


