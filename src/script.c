
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "script.h"
#include "utilities.h"

script_t *importScript(int8_t *path) {
    script_t *output = malloc(sizeof(script_t));
    output->path = mallocRealpath(path);
    output->body = readEntireFile(&(output->bodyLength), output->path);
    if (output->body == NULL) {
        free(output);
        return NULL;
    }
    return output;
}


