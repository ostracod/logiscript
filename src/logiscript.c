
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "vector.h"
#include "script.h"
#include "resolve.h"
#include "error.h"
#include "testSocket.h"

void printUsage() {
    printf("Usage:\nlogiscript [file path]\n");
}

int main(int argc, const char *argv[]) {
    if (argc != 2 && argc != 3) {
        printUsage();
        return 1;
    }
    int8_t *scriptPath;
    if (argc == 3) {
        if (strcmp(argv[1], "--socket") != 0) {
            printUsage();
            return 1;
        }
        isInSocketMode = true;
        setbuf(stdout, NULL);
        int8_t *socketPath = mallocRealpath((int8_t *)(argv[2]));
        if (socketPath == NULL) {
            printf("ERROR: Could not find socket file!\n");
            return 1;
        }
        int8_t tempResult = connectToTestSocket(socketPath);
        free(socketPath);
        if (!tempResult) {
            printf("Could not connect to socket!\n");
            return 1;
        }
        writeTextToTestSocket((int8_t *)"entryPoint");
        scriptPath = readFromTestSocket(NULL);
    } else {
        isInSocketMode = false;
        scriptPath = (int8_t *)(argv[1]);
    }
    hasThrownError = false;
    createEmptyVector(&scriptList, sizeof(script_t *));
    initializeNumberConstants();
    importEntryPointScript(scriptPath);
    if (hasThrownError) {
        printStackTrace();
        return 1;
    }
    return 0;
}


