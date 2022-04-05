#include "server.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "Packet.h"
#include "Serialize.h"
#include "Queue.h"

pthread_mutex_t mutex;
struct sockaddr_in ServerInfo;
int ResultSocket;
struct hostent Host;
char Buf[SIZE_MESSAGE_PACKET];

struct Client
{
    char Name[5];
    char IPAddress[16];
    int SocketID;
    pthread_t thread;
    char Buffer[SIZE_MESSAGE_PACKET];
};
struct ClientList
{
    struct Client Data;
    struct ClientList *next;
}*client, *Current;

void InsertClient(char name[5], char IP[16], int socket, thrd_t th)
{
    struct ClientList *node = (struct ClientList*) malloc(sizeof(struct ClientList));

    strcpy(node->Data.Name, name);
    strcpy(node->Data.IPAddress, IP);
    node->Data.SocketID = socket;
    node->Data.thread = th;
    node->next = client;
    client = node;

}


//delete a link with given socketID
struct ClientList* delete(int socketID) {

   //start from the first link
   struct ClientList* current = client;
   struct ClientList* previous = NULL;
	
   //if list is empty
   if(client == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->Data.SocketID != socketID) {

      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == client) {
      //change first to point to next link
      client = client->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}


void start()
{
    if(CreateSocket())
    {
        if(InitServer())
        {
            pthread_mutex_init(&mutex, NULL);
            int i = 0;
            while(true)
            {
                if(i < MAX_CLIENT)
                {
                    struct timespec time1, time2;
                    time1.tv_sec = 0;
                    time2.tv_sec = 1;
                    thrd_sleep(&time1, &time2);

                    listen(ResultSocket, MAX_CLIENT);

                    socklen_t size = sizeof(ServerInfo);
                    int TempSocket;

                    TempSocket = accept(ResultSocket, (struct sockaddr*) &ServerInfo, &size);
                    printf("Hello Welcome to Server\n\n");

                    NewClient(TempSocket);
                    pthread_create(&client->Data.thread, NULL, (void*)Recv, &client->Data.SocketID);
                    i++;
                }  
            }
        }
        else{
            exit(0);
        }
    }
    else{
        exit(0);
    }
}

bool CreateSocket()
{
    ResultSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(ResultSocket == INVALID_SOCKET)
    {
        printf("Error Socket : %s\n", strerror(errno));
        return false;
    }
    else
        return true;
}

bool InitServer()
{
    ServerInfo.sin_family = AF_INET;
    ServerInfo.sin_addr.s_addr = inet_addr("192.168.1.38");
    ServerInfo.sin_port = htons(PORT);

    if(INVALID_SOCKET == bind(ResultSocket, (struct sockaddr*) &ServerInfo, sizeof(ServerInfo)))
    {
        printf("Error Bind Socket : %s\n", strerror(errno));
        return false;
    }
    else
        return true;
}


void SendMsg(struct MessagePacket packet, int socketID)
{
    struct ClientList *node = client;
    SerializeMessage(Buf, packet);
    while(node != NULL)
    {
        if(node->Data.SocketID != socketID)
        send(node->Data.SocketID, Buf, SIZE_MESSAGE_PACKET, 0);
        node = node->next;
    }
}

void Recv(int *socketID)
{    
    while (true)
    {
        struct timespec time1, time2;
        time1.tv_sec = 0;
        time2.tv_sec = 1;

        thrd_sleep(&time1, &time2);
        struct ClientList *temp;

        memset(Buf, '\0', SIZE_MESSAGE_PACKET);
        
        if(0 > recv(*socketID, &Buf, SIZE_MESSAGE_PACKET, 0))
        {
            printf("Error Recv : %s\n", strerror(errno));
        }
        else{
            insert(*socketID);
            pthread_mutex_lock(&mutex);
            Current = client;

            while (Current != NULL)
            {
                if(Current->Data.SocketID == *socketID)
                {
                    memcpy(Current->Data.Buffer, Buf, SIZE_MESSAGE_PACKET);
                    temp = Current;
                }
                
                Current = Current->next;
            }    
        }

        struct MessagePacket packet;
        DeserializeMessage(temp->Data.Buffer, &packet);
        
        switch (packet.header)
        {
        case Message:
            SendMsg(packet, removeData());
            pthread_mutex_unlock(&mutex);
            break;

        case Chunck:
            SendMsg(packet, removeData());
            pthread_mutex_unlock(&mutex);
            break;

        case Bye:
            {
                delete(peek());
                if(shutdown(removeData(), SHUT_RDWR) != 0)
                {
                    printf("Error Socket Shutdown : %s\n", strerror(errno));
                }
                else
                    printf("The socket was turned off.\n");

                pthread_mutex_unlock(&mutex);
                if(client == NULL)
                {
                    shutdown(ResultSocket, SHUT_RDWR);
                    exit(0);
                }
                pthread_exit(NULL);
            }
            break;
        
        default:
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
}

void NewClient(int socketID)
{
    char buffer[SIZE_MESSAGE_PACKET];
    struct MessagePacket packet;

    memset(buffer, '\0', SIZE_MESSAGE_PACKET);
    if( 0 > recv(socketID, &buffer, SIZE_MESSAGE_PACKET, 0))
    {
        printf("Error Can't Recv Packet NewClient : %s\n", strerror(errno));
        if(shutdown(socketID, SHUT_RDWR) != 0)
        {
            printf("Error Can't Shutdown socket : %s\n", strerror(errno));
            exit(0);
        }
    }
    else{
        DeserializeMessage(buffer, &packet);
        if(packet.header == Newclient)
        {
            InsertClient(packet.Name, packet.IP, socketID, (thrd_t) malloc(sizeof(thrd_t)));
            printf("IP : %s\n", client->Data.IPAddress);
            printf("name : %s\n", client->Data.Name);

        }
        else{
            printf("Error Bad Packet\n");
            exit(0);
        }  
    }
}
