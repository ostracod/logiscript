
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "script.h"
#include "resolve.h"
#include "error.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        printf("Usage:\nlogiscript [file path]\n");
        return 1;
    }
    initializeNumberConstants();
    int8_t *scriptPath = (int8_t *)(argv[1]);
    importScript(scriptPath);
    if (hasThrownError) {
        printStackTrace();
        return 1;
    }
    return 0;
}


