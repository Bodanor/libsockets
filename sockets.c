#include "sockets.h"

/**
 * @file sockets.c
 * @author Liwinux & Tinmar1010
 * @brief Function declarations for the socket library.
 * @version 0.1
 * @date 2023-10-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/* Packed is used to avoid compilator optimizations to be applied */
typedef struct __attribute__((packed)) header_t{
    uint8_t next_packet_flag;
    uint32_t body_size;

} Header ;

/* Packed is used to avoid compilator optimizations to be applied */
typedef struct __attribute__((packed)) packet_t {
    Header header;
    uint8_t raw_body_data[PACKET_BODY_MAX_SIZE];

} Packet;


/**
 * @brief Create a socket.
 * 
 * @param ip Ip address to assign to the socket.
 * @param port Port nulber to assign to the socket.
 * @param results pointer to pointer to a struct addrinfo that contains all the socket info.
 * @return return the newly created socket. 
 */
static int create_socket(const char *ip, const int port, struct addrinfo **results);

/**
 * @brief Create a msg pointer. If the first parameter is not NULL, the packet
 * will we concatenated to the alreay created message.
 * 
 * @param message a pointer to pointer to a Message struct that will hold the newly
 * created message.
 * @param packet a pointer to a packet struct that hold the header + the body.
 * @return Message* : a pointer to a Message struct that hold the data.
 */
static Message* create_msg(Message **message, const Packet *packet);

void destroyMessage(Message *message)
{
    free(message->data);
    free(message);
}

Message* create_msg(Message **message, const Packet *packet)
{
    /* If the packet is invalid we simply return return nothing .*/
    /* This is valid as if the first condition is true, the next condition are not evaluated */
    if (packet->header.body_size <= 0 ) {
        return NULL;
    }
    /* If the pointer is NULL, we allocate it and copy the body from the packet inside the message structure */
    if (*message == NULL) {
        *message = (Message*)malloc(sizeof(Message));
        if (*message == NULL)
            return NULL;
        
        (*message)->data = (uint8_t*)malloc(sizeof(uint8_t)*packet->header.body_size);
        if ((*message)->data == NULL)
            return NULL;
        memcpy((*message)->data, packet->raw_body_data, packet->header.body_size); /* Copy the data inside the message */
        (*message)->data_size = packet->header.body_size; /* Don't forget to update the message size */
    }
    /* If it is not null, we simply concatenate the packet with the existing message */
    else {
        /* Reallocate with the previous message size + the new packet body size */
        (*message)->data = (uint8_t*)realloc((*message)->data, (*message)->data_size + packet->header.body_size);
        if ((*message)->data == NULL)
            return NULL;
        memcpy(&(*message)->data[(*message)->data_size], packet->raw_body_data, packet->header.body_size); /* Copy the data inside the message *from previous msg */
        (*message)->data_size = (*message)->data_size + packet->header.body_size; /* Don't forget to update the message size */
    }

    return *message;
    
}


/* Function to generate a socket for the server or the client */

int create_socket(const char *ip, const int port, struct addrinfo **results)
{
    int server_socket;
    struct addrinfo hints;
    char buffer[50]; /* sprintf port converter buffer */

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket error !");
        exit(1);
    }
#ifdef DEBUG
    printf("[DEBUG] Socket created 1\n");
#endif
    
    /* Reutilisable socket */
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        perror("Setsockopt Error !");
        exit(1);
    }

    /* We fill in the socket address to bind */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    sprintf(buffer, "%d", port);
    
    if (ip == NULL)
    {
        if (getaddrinfo(NULL, buffer, &hints, &(*results)) != 0)
            exit(1);
    }

    else
    {
        if (getaddrinfo(ip, buffer, &hints, &(*results)) != 0) 
            exit(1);
    }
        
#ifdef DEBUG
    printf("[DEBUG] getaddrinfo successfull !\n");
#endif

    return server_socket;
}

int Create_server(int port)
{
    struct addrinfo *results;
    int server_socket;

    results = NULL;
#ifdef DEBUG
    char hosts[NI_MAXHOST];
    char ports[NI_MAXSERV];
#endif

    results = NULL;
    server_socket = create_socket(NULL, port, &results);

    if (bind(server_socket, results->ai_addr, results->ai_addrlen) < 0) {
        perror("Socket error !");
        exit(1);
    }
    
#ifdef DEBUG
    getnameinfo(results->ai_addr, results->ai_addrlen, hosts, NI_MAXHOST, ports, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
    printf("[DEBUG] Socket binded with IP : %s on PORT : %s\n", hosts, ports);
#endif

    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("Socket error !");
        exit(1);
    }

#ifdef DEBUG
    printf("[DEBUG] Listening on IP : %s on PORT : %s\n", hosts, ports);
#endif

    freeaddrinfo(results);
    return server_socket;

}
int Server_connect(const char *server_ip, const int port)
{
    struct addrinfo *results;
    int client_socket;
    
    results = NULL;
#ifdef DEBUG
    char hosts[NI_MAXHOST];
    char ports[NI_MAXSERV];
#endif

    results = NULL;
    client_socket = create_socket(server_ip, port, &results);

    if (connect(client_socket, results->ai_addr, results->ai_addrlen) == -1) {
        perror("Erreur de connect ! ");
        exit(1);
    }

#ifdef DEBUG
    getnameinfo(results->ai_addr, results->ai_addrlen, hosts, NI_MAXHOST, ports, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
    printf("[DEBUG] Connected to server with IP : %s on PORT : %s\n", hosts, ports);
#endif

    freeaddrinfo(results);
    return client_socket;
}
int Send_msg(const int src_socket, const uint8_t *data ,const uint64_t longMessage)
{

#ifdef DEBUG
    struct sockaddr_in adrClient;
    socklen_t adrClientLen;
    char hosts[NI_MAXHOST];
    char ports[NI_MAXSERV];
#endif

    Packet current_packet; /* Current packet to be sent. Will be filled in when needed */
    int step = 0;   /* Used to count of many times the message fits in PACKET_BODY_MAX_SIZE chuncks */
    uint64_t bytes_send = 0; /* Used to know if we have sent all the bytes to the socket. Main loop */
    int write_sent = 0;

#ifdef DEBUG
    /* Code permettant de connaitre les informations sur le client connecté*/
    adrClientLen = sizeof(struct sockaddr_in); 
    getpeername(src_socket,(struct sockaddr*)&adrClient,&adrClientLen);
    getnameinfo((struct sockaddr*)&adrClient,adrClientLen,hosts,NI_MAXHOST,ports,NI_MAXSERV,NI_NUMERICSERV | NI_NUMERICHOST);
#endif

    /* We no data to send then we simply return from the function */
    if (data == NULL || longMessage <= 0) {
        return -2; /* Bad parameters passed to this function */
    }
    
    /* While not all data has been sent, we loop */
    while (bytes_send < longMessage) {
        
        /* Is the current message too big to fit in one single packet ? */
        if (longMessage - (step * PACKET_BODY_MAX_SIZE) > PACKET_BODY_MAX_SIZE) {
            
            /* We create a packet by setting the next packet flag and fill in the whole body */
            current_packet.header.next_packet_flag = 1;
            current_packet.header.body_size = PACKET_BODY_MAX_SIZE;
            memcpy(current_packet.raw_body_data, data, PACKET_BODY_MAX_SIZE);
            step++;
        }
        else {
            /* If it fits inside a packet we unset the flag and copy the body */
            current_packet.header.next_packet_flag = 0;
            current_packet.header.body_size = longMessage - (step * PACKET_BODY_MAX_SIZE);
            memcpy(current_packet.raw_body_data, data, longMessage - (step * PACKET_BODY_MAX_SIZE));
            
        }
        
        /* Finally, we send to the socket the current packet as size : Body size + sizeof a header.
        ** We get in return the bytes sent without the header, effectively getting the raw data size.  
        */
        write_sent = write(src_socket, &current_packet, current_packet.header.body_size + sizeof(current_packet.header)) - sizeof(current_packet.header);
        if (write_sent == 0){
#ifdef DEBUG
            printf("[DEBUG] Client on socket --> {%s} on PORT : %s is now disconnected !\n",hosts,ports);
#endif
            return -1;
        }
        else if (write_sent == -1) {
#ifdef DEBUG
            printf("[DEBUG] Write error on socket --> {%s} on PORT : %s !\n",hosts,ports);
#endif
        return -3;
        }
        /* Increment the bytes that have been sent. */
        bytes_send += write_sent;

#ifdef DEBUG
    printf("[DEBUG] %d BYTES sent to socket --> {%s} on PORT : %s\n",current_packet.header.body_size + SIZE_OF_HEADER,hosts,ports);
#endif

    }
    /* Check if every has been sent */
    
    return bytes_send;

}

/* 0 disconnected
 * -1 read error
 * -2 header or body mismatch
*/

int Receive_msg(const int src_socket, Message **message)
{    
    Packet current_packet;
    int64_t recv_bytes;

    *message = NULL;
    


#ifdef DEBUG
    /* Code permettant de connaitre les informations sur le client connecté*/
    struct sockaddr_in adrClient;
    socklen_t adrClientLen;

    char hosts[NI_MAXHOST];
    char ports[NI_MAXSERV];
    adrClientLen = sizeof(struct sockaddr_in); 
    getpeername(src_socket,(struct sockaddr*)&adrClient,&adrClientLen);
    getnameinfo((struct sockaddr*)&adrClient,adrClientLen,hosts,NI_MAXHOST,ports,NI_MAXSERV,NI_NUMERICSERV | NI_NUMERICHOST);
#endif

    
    do 
    {
        /* Read the whole header */
        recv_bytes = read(src_socket, &current_packet.header, sizeof(Header));
        if(!recv_bytes) {
#ifdef DEBUG
            printf("[DEBUG] Client on socket --> {%s} on PORT : %s is now disconnected !\n",hosts,ports);
#endif
            return -1;
        }
        else if (recv_bytes == -1) {
#ifdef DEBUG
            printf("[DEBUG] Read error on socket --> {%s} on PORT : %s !\n",hosts,ports);
#endif
            return -3;
        }
        else if (recv_bytes != sizeof(Header)) {
#ifdef DEBUG
            printf("[DEBUG] Header corrupted on socket --> {%s} on PORT : %s !\n",hosts,ports);
            return -2;
#endif
        }

        recv_bytes = read(src_socket, current_packet.raw_body_data, current_packet.header.body_size);
        if (!recv_bytes) {
#ifdef DEBUG
            printf("[DEBUG] Client on socket --> {%s} on PORT : %s is now disconnected !\n",hosts,ports);
            return -1;
#endif
        }
        else if (recv_bytes == -1) {
#ifdef DEBUG
            printf("[DEBUG] Read error on socket --> {%s} on PORT : %s !\n",hosts,ports);
#endif
            return -3;
        }
        else if (recv_bytes != current_packet.header.body_size) {
#ifdef DEBUG
            printf("[DEBUG] Body corrupted on socket --> {%s} on PORT : %s !\n",hosts,ports);
            return -2;
#endif
        }

#ifdef DEBUG
    printf("[DEBUG] Received %d(body) %ld(header) BYTES from socket --> {%s} on PORT : %s\n",current_packet.header.body_size, sizeof(Header),hosts,ports);
    printf("[DEBUG] Next packet FLAG is : %d\n", current_packet.header.next_packet_flag);
#endif

        create_msg(message, &current_packet);
    }while(current_packet.header.next_packet_flag);
    
    
    
    return (*message)->data_size;
}

int Accept_connexion(int server_socket)
{
#ifdef DEBUG
    struct sockaddr_in client_address;
    socklen_t client_addr_lenght;
    char hosts[NI_MAXHOST];
    char ports[NI_MAXSERV];
#endif

    int new_client_socket;

    if ((new_client_socket = accept(server_socket, NULL, NULL)) == -1) {
        perror("Accept error !");
        exit(1);
    }


#ifdef DEBUG
    client_addr_lenght = sizeof(struct sockaddr_in);
    getpeername(new_client_socket, (struct sockaddr *)&client_address, &client_addr_lenght);
    getnameinfo((struct sockaddr *)&client_address, client_addr_lenght, hosts, NI_MAXHOST, ports, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

    printf("[DEBUG] New client {%s} connected on port : %s\n", hosts, ports);
#endif

    return new_client_socket;
}

int close(int src_socket)
{
    shutdown(src_socket, SHUT_RDWR);
    return 0;
}