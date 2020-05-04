
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "vector.h"
#include "script.h"
#include "resolve.h"
#include "error.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        printf("Usage:\nlogiscript [file path]\n");
        return 1;
    }
    hasThrownError = false;
    createEmptyVector(&scriptList, sizeof(script_t *));
    initializeNumberConstants();
    int8_t *scriptPath = (int8_t *)(argv[1]);
    importEntryPointScript(scriptPath);
    if (hasThrownError) {
        printStackTrace();
        return 1;
    }
    return 0;
}


