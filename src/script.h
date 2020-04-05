
#ifndef SCRIPT_HEADER_FILE
#define SCRIPT_HEADER_FILE

typedef struct script {
    int8_t *path;
    int8_t *body;
    int64_t bodyLength;
} script_t;

script_t *importScript(int8_t *path);

// SCRIPT_HEADER_FILE
#endif


