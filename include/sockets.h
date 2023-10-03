#ifndef __SOCKETS_H__
#define __SOCKETS_H__

/**
 * @file sockets.h
 * @author Liwinux & Tinmar1010
 * @brief Function defintions for the socket library.
 * @version 0.1
 * @date 2023-10-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

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

/**
 * @brief Create a new server socket.
 * 
 * @param port port to listen to.
 * @return the server socket created.
 */
int Create_server(const int port);

/**
 * @brief Connect to the server socket.
 * 
 * @param server_ip The server ip to connect to. 
 * @param port The server port number to connect to.
 * @return The connection socket.
 */
int Server_connect(const char *server_ip, const int port);

/**
 * @brief Accept a new connection from the src socket.
 * 
 * @param src_socket The socket to accept a connection from.
 * @return The newly created socket from the accept call function.
 */
int Accept_connexion(const int src_socket);

/**
 * @brief Send a message to the dest_socket socket.
 * 
 * @param dest_socket The socket to send a message.
 * @param data The data to send to the socket.
 * @param data_size The size of the data to send.
 * @return The number of bytes sent without the header size of each packet.
 * @return -1 : If the dest_socket has been disconnected.
 * @return -2 : Bad parameter passed to the function. 
 * @return -3 : If the socket is broken and a write could not be achieved.
 */
int Send_msg(const int dest_socket, const uint8_t *data, uint64_t data_size);

/**
 * @brief Receive a message from the src_socket.
 * Message **message has to point to NULL as this function will set it to NULL
 * anyway.
 * 
 * @param src_socket The source socket to receive a message from.
 * @param message a pointer to pointer to a Message type that has to be set
 * to NULL. Will hold the data.
 * @return The bytes received wihtout the header of each packet.
 * @return -1 : If the src_socket has been disconnected.
 * @return -2 : If the header or message is corrupted and thus data becomes invalid.
 * @return -3 : If the socket is broken and a read could not be achieved.
 */
int Receive_msg(const int src_socket, Message **message);

/**
 * @brief Deallocate memory for a message previously received.
 * 
 * @param message The message to destroy.
 */
void destroyMessage(Message *message);

/**
 * @brief Close the connection to a provided socket.
 * 
 * @param src_socket The socket issue a close to.
 * @return this function should always return 0.
 */
int close(int src_socket);

#endif