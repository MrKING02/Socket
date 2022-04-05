/*  In the server, packet that are serialized by the clients and sent to the server.
    Are DeSerialized by the server.
    If the packet received is a NewClient packet, 
    it stores the new client information in the data structure, which is of the link list type.
    And if the packet header is bye type, it deletes the client information from the data structure and
    the corresponding socket that closes the client and terminates its thread.
    And if it is other than the above, it will send it to other clients.
    
*/
/*
    struct Client
    {
        char Name[5];                       // client Name.
        char IPAddress[16];                 // IP Client.
        int SocketID;                       // The socket to which that client is connected is stored in this.
        thrd_t thread;                      // The client thread is stored in this.
        char Buffer[SIZE_MESSAGE_PACKET];
    }
    struct ClientList
    {
        struct Client Data;
        struct ClientList *next; // This is a node.
    }
*/

#ifndef SERVER_H
#define SERVER_H

#include <threads.h>
#include "Packet.h"

#define PORT 8080
#define MAX_CLIENT 10
#define INVALID_SOCKET -1

struct Client;
struct ClientList;

void InsertClient(char [5], char [16], int, thrd_t);
struct ClientList* delete(int);
void start();
bool CreateSocket();
bool InitServer();
void SendMsg(struct MessagePacket, int);
void Recv(int *);
void NewClient(int);


#endif




