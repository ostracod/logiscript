
#ifndef TEST_SOCKET_HEADER_FILE
#define TEST_SOCKET_HEADER_FILE

int8_t connectToTestSocket(int8_t *path);
void writeToTestSocket(int8_t *data, int32_t length);
int8_t *readFromTestSocket(int32_t *lengthDestination);

// TEST_SOCKET_HEADER_FILE
#endif


