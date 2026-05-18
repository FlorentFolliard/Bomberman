#ifndef PROTOCOL_H
#define PROTOCOL_H

#define DEFAULT_PORT 12345
#define BUFFER_SIZE 512

typedef struct {
    int client_id;
    char message[BUFFER_SIZE];
} TestPacket;

#endif