
#ifndef EVALUATE_HEADER_FILE
#define EVALUATE_HEADER_FILE

#include "script.h"
#include "statement.h"

int32_t maximumMarkAndSweepDelay;

void evaluateStatement(heapValue_t *frame, baseStatement_t *statement);
void evaluateScript(script_t *script);

// EVALUATE_HEADER_FILE
#endif


