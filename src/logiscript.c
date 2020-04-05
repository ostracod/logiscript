
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "script.h"

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        printf("Usage:\nlogiscript [file path]\n");
        return 1;
    }
    int8_t *scriptPath = (int8_t *)(argv[1]);
    script_t *script = importScript(scriptPath);
    if (script == NULL) {
        printf("Script file is missing!\n");
        return 1;
    }
    printf("Script contents:\n%s\n", script->body);
    return 0;
}


