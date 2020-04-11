
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wordexp.h>
#include "utilities.h"

int8_t *mallocText(int8_t *text, int64_t length) {
    int8_t *output = malloc(length + 1);
    memcpy(output, text, length);
    output[length] = 0;
    return output;
}

int8_t *mallocRealpath(int8_t *path) {
    int64_t tempLength = strlen((char *)path);
    int8_t *tempText = malloc((tempLength + 1) * 2);
    int64_t tempIndex = 0;
    for (int64_t index = 0; index < tempLength; index++) {
        int8_t tempCharacter = path[index];
        if (tempCharacter == ' ') {
            tempText[tempIndex] = '\\';
            tempIndex += 1;
        }
        tempText[tempIndex] = tempCharacter;
        tempIndex += 1;
    }
    tempText[tempIndex] = 0;
    wordexp_t expResult;
    wordexp((char *)tempText, &expResult, 0);
    free(tempText);
    int8_t *output = (int8_t *)realpath(expResult.we_wordv[0], NULL);
    wordfree(&expResult);
    return output;
}

int8_t *readEntireFile(int64_t *length, int8_t *path) {
    FILE *fileHandle = fopen((char *)path, "r");
    if (fileHandle == NULL) {
        return NULL;
    }
    fseek(fileHandle, 0, SEEK_END);
    int64_t fileLength = ftell(fileHandle);
    int8_t *output = malloc(fileLength + 1);
    fseek(fileHandle, 0, SEEK_SET);
    fread(output, 1, fileLength, fileHandle);
    output[fileLength] = 0;
    fclose(fileHandle);
    if (length != NULL) {
        *length = fileLength;
    }
    return output;
}


