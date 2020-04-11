
#ifndef UTILITIES_HEADER_FILE
#define UTILITIES_HEADER_FILE

#define true 1
#define false 0

int8_t *mallocText(int8_t *text, int64_t length);
int8_t *mallocRealpath(int8_t *path);
int8_t *readEntireFile(int64_t *length, int8_t *path);

// UTILITIES_HEADER_FILE
#endif


