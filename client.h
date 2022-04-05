/*  This file contains the client code..    */
/*  The data structures used are of the list link type  */
/*  ----------struct---------------*/
/*                                                      */
/*  struct ClientMessageList{                           */
/*      struct ClientMessage message;  // This is a data structure for the message.
        struct ClientMessageList *next;    This is a node.                                 
    }                               */
/*  struct ClientMessage {
        int ID; // This is the ID of each message
        int Status; // This status specifies the incoming or outgoing message.
        char IP[16]; // In this IP, the sender or receiver is stored.
        char Name[5]; // In this Name, the sender or receiver is stored.
        char Buf1[MAX_BUFFER]; // This is for the first part of the message.
        char Buf2[MAX_BUFFER]; // This is for the second part of the message, but if the message is two-part, otherwise it will be null.
        char TimeInfo[256]; // Receiving or sending messages is saved at this time.    */
/*  }                                                       */
/**/
/**/
/**/
/**/
/**/
/**/
/**/

#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

#define PORT 8080
#define INVALID_SOCKET -1
#define MAX_BUFF 80
#define INPUT 0
#define OUTPUT 1


struct ClientMessage;
struct ClientMessageList;

void SetName();
void SetServer();
void start();
bool CreateSocket();
void SendMsg();
void Recv();
void SendNewClientMessage();
char* GetIP();
void InsertMessage(struct ClientMessage);
struct ClientMessageList* delete(int);
void ShowMessage();



#endif

