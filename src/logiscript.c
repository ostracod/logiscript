
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "utilities.h"
#include "vector.h"
#include "script.h"
#include "resolve.h"
#include "error.h"
#include "testSocket.h"
#include "value.h"

void printUsage() {
    printf("Usage:\nlogiscript [file path]\nlogiscript --version\n");
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
        writeTextToTestSocket((int8_t *)"garbageCollection");
        int8_t *tempText = readFromTestSocket(NULL);
        int32_t tempMode;
        sscanf((char *)tempText, "%d", &tempMode);
        free(tempText);
        setGarbageCollectionMode(tempMode);
        writeTextToTestSocket((int8_t *)"entryPoint");
        scriptPath = readFromTestSocket(NULL);
    } else {
        isInSocketMode = false;
        scriptPath = (int8_t *)(argv[1]);
        if (strcmp((char *)scriptPath, "--version") == 0) {
            printf("1.0.0\n");
            return 0;
        }
    }
    hasThrownError = false;
    thrownErrorValue.type = VALUE_TYPE_VOID;
    createEmptyVector(&scriptList, sizeof(script_t *));
    initializeNumberConstants();
    importEntryPointScript(scriptPath);
    if (isInSocketMode) {
        int8_t *messageText;
        asprintf((char **)&messageText, "heapCount %" PRId64, getHeapAllocationCount());
        writeTextToTestSocket(messageText);
        free(messageText);
    }
    if (hasThrownError) {
        printStackTrace();
        return 1;
    }
    return 0;
}


