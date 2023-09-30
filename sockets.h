#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PACKET_BODY_MAX_SIZE 1019
#define SIZE_OF_HEADER 5


typedef struct __attribute__((packed)) msg_t{
    uint64_t data_size;
    uint8_t *data;
}Message ;

int Create_server(const int port);
int Server_connect(const char *server_ip, const int port);
int Accept_connexion(const int src_socket);
int Send_msg(const int dest_socket, const uint8_t *data, uint64_t data_size);
int Receive_msg(const int src_socket, Message **message);
void destroyMessage(Message *message);
int close(int src_socket);

#endif