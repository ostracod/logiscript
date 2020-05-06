
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "utilities.h"
#include "testSocket.h"

int32_t socketHandle;

int8_t connectToTestSocket(int8_t *path) {
    socketHandle = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sun_family = AF_UNIX;
    strncpy(sockAddr.sun_path, (char *)path, sizeof(sockAddr.sun_path) - 1);
    int32_t tempResult = connect(
        socketHandle,
        (struct sockaddr *)&sockAddr,
        sizeof(sockAddr)
    );
    return (tempResult == 0);
}

void writeToTestSocket(int8_t *data, int32_t length) {
    write(socketHandle, &length, sizeof(length));
    write(socketHandle, data, length);
}

int8_t *readFromTestSocket(int32_t *lengthDestination) {
    int32_t tempLength;
    read(socketHandle, &tempLength, sizeof(tempLength));
    if (lengthDestination != NULL) {
        *lengthDestination = tempLength;
    }
    int8_t *output = malloc(tempLength);
    read(socketHandle, output, tempLength);
    return output;
}


