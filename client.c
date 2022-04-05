#include "client.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "Packet.h"
#include "Serialize.h"
#include "UI.h"

struct sockaddr_in ServerInfo;
int ResultSocket, client;
char Buf[MAX_BUFF];
char Name[5];
char ServerAddress[20];

struct ClientMessage
{
    int ID;
    int Status;
    char IP[16];
    char Name[5];
    char Buf1[MAX_BUFFER];
    char Buf2[MAX_BUFFER];
    char TimeInfo[256];
};
struct ClientMessageList
{
    struct ClientMessage message;
    struct ClientMessageList *next;
}*HeadMessage, *CurrentMessage;

void SetName()
{   
    while (true)
    {
        mvwprintw(Input, 1, 1, "%s", "Enter Your Name ( Name < 5) : ");
        wrefresh(Input);

        wgetstr(Input, Name);

        if(strlen(Name) <= 5 && strlen(Name) > 1)
            break;

        wclear(Input);
        draw_borders(Input);
    }
}

void SetServer()
{
    wclear(Input);
    draw_borders(Input);
    mvwprintw(Input, 1, 1, "%s", "Enter Server Name or Address : ");
    wrefresh(Input);
    
    wgetstr(Input, ServerAddress);  
}

void start()
{   
    SetName();
    if(CreateSocket())
    {
        SendNewClientMessage();

        pthread_t thrdSend, thrdRecv;

        pthread_create(&thrdSend, NULL, (void*)SendMsg, NULL);
        pthread_create(&thrdRecv, NULL, (void*)Recv, NULL);

        pthread_join(thrdSend, NULL);
        pthread_join(thrdRecv, NULL);
    }
    else{
        sleep(5);
        CleanUP_UI();
        exit(0);
    }
}
bool CreateSocket()
{
    ResultSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(ResultSocket == INVALID_SOCKET)
    {
        wprintw(Output, "%s%s\n", "Error : ", strerror(errno));
        return false;
    }

    bool isOK = false;
    for(int i = 0; i < 4 && isOK != true; i++)
    {
        SetServer();
        ServerInfo.sin_family = AF_INET;
        if(isalpha(ServerAddress[0]) != 0)
            ServerInfo.sin_addr.s_addr = inet_aton(ServerAddress, NULL);
        else
            ServerInfo.sin_addr.s_addr = inet_addr(ServerAddress);
        ServerInfo.sin_port = htons(PORT);

        int Res = connect(ResultSocket, (struct sockaddr*) &ServerInfo, sizeof(ServerInfo));
        if(Res == INVALID_SOCKET)
        {
            mvwprintw(Output, 1, 1, "%s%s\n", "Error : ", strerror(errno));
            wrefresh(Output);
        }
        else
            isOK = true;
    }

    if(isOK)
        return true;
    return false;
}

void SendMsg()
{
    while (true)
    {
        wclear(Input);
        draw_borders(Input);
        mvwprintw(Input, 1, 1, "%s", "> ");
        wrefresh(Input);
        memset(Buf, '\0', MAX_BUFF);
        wgetstr(Input, Buf);

        struct MessagePacket packet;
        struct ClientMessage message;
        memset(message.Buf1, '\0', MAX_BUFFER);
        memset(message.Buf2, '\0', MAX_BUFFER);
        struct tm *t; time_t rawTime;

        time(&rawTime);
        t = localtime(&rawTime);
        strftime(message.TimeInfo, 256, "%T", t);

        if(strcmp(Buf, "bye") == 0)
            packet.header = Bye;
        else
        {
            packet.header = Message;
        }

        if(strlen(Buf) < MAX_BUFFER)
        {
            packet.Size = strlen(Buf) + 1;
            strcpy(packet.Buffer, Buf);
            strcpy(packet.IP, GetIP());
            strcpy(packet.Name, Name);
            packet.isEnd = true;

            // initialize clientMessage ...
            strcpy(message.Buf1, packet.Buffer);
            strcpy(message.IP, packet.IP);
            strcpy(message.Name, packet.Name);

            if(HeadMessage == NULL)
                message.ID = 1;
            else
                message.ID = HeadMessage->message.ID + 1;

            message.Status = OUTPUT;

            InsertMessage(message);
            strcpy(HeadMessage->message.Buf2, message.Buf2); 
            // end initialize ClientMessage... 

            char Buffer[SIZE_MESSAGE_PACKET];
            SerializeMessage(Buffer, packet);

            if(0 > send(ResultSocket, Buffer, SIZE_MESSAGE_PACKET, 0))
            {
                mvwprintw(Output, 1, 1, "%s%s\n", "Error Send : ", strerror(errno));
                wrefresh(Output);
            }
        }
        else{ // chunck
            char buf1[MAX_BUFFER], buf2[MAX_BUFFER];
            strncpy(buf1, Buf, MAX_BUFFER - 1);
            for(int i = 0, j = 39; j < 80; i++, j++)
            {
                buf2[i] = Buf[j];
            }

            // initialize Message Packet...
            strcpy(packet.Buffer, buf1);
            packet.isEnd = false;
            packet.Size = strlen(buf1) + 1;
            strcpy(packet.IP, GetIP());
            strcpy(packet.Name, Name);
            char Buffer[SIZE_MESSAGE_PACKET];
            SerializeMessage(Buffer, packet);
            // End initialize Message Packet. .. 

            strcpy(message.Buf1, buf1);
            strcpy(message.IP, packet.IP);
            strcpy(message.Name, packet.Name);
            message.Status = OUTPUT;
            InsertMessage(message);
            strcpy(HeadMessage->message.Buf2, buf2);

            if( 0 > send(ResultSocket, Buffer, SIZE_MESSAGE_PACKET, 0))
            {
                mvwprintw(Output, 1, 1, "%s%s\n", "Error Send : ", strerror(errno));
                wrefresh(Output);
            }
            else{
                memset(packet.Buffer, '\0', MAX_BUFFER);
                memset(Buffer, '\0', SIZE_MESSAGE_PACKET);
                packet.header = Chunck;
                strcpy(packet.Buffer, buf2);
                packet.isEnd = true;
                packet.Size = strlen(buf2) + 1;
                SerializeMessage(Buffer, packet);
                if( 0 > send(ResultSocket, Buffer, SIZE_MESSAGE_PACKET, 0))
                {
                    mvwprintw(Output, 1, 1, "%s%s\n", "Error Send Chunck : ", strerror(errno));
                    wrefresh(Output);
                }
            }          
        }
        ShowMessage();

        if(packet.header == Bye)
            if(shutdown(ResultSocket, SHUT_RDWR) != 0)
            {
                mvwprintw(Output, 1, 1, "%s%s\n", "Error Shutdown Socket : ", strerror(errno));
                wrefresh(Output);
            }
            else
                break;
    }
    CleanUP_UI();
    exit(0);
}
void Recv()
{
    while(true)
    {
        char buffer[SIZE_MESSAGE_PACKET];
        memset(buffer, '\0', SIZE_MESSAGE_PACKET);

        if( 0 > recv(ResultSocket, &buffer, SIZE_MESSAGE_PACKET, 0))
        {
            mvwprintw(Output, 1, 1, "%s%s\n", "Error Recv : ", strerror(errno));
            wrefresh(Output);
        }
        else
        {
            struct ClientMessage message;
            memset(message.Buf1, '\0', MAX_BUFFER);
            memset(message.Buf2, '\0', MAX_BUFFER);
            struct MessagePacket packet;
            DeserializeMessage(buffer, &packet);
            
            if(HeadMessage == NULL)
                message.ID = 1;
            else
                message.ID = HeadMessage->message.ID + 1;

            if(packet.header == Message)
            {
                strcpy(message.Buf1, packet.Buffer);
                strcpy(message.IP, packet.IP);
                strcpy(message.Name, packet.Name);
                struct tm *t; time_t rawTime;
                time(&rawTime);
                t = localtime(&rawTime);
                strftime(message.TimeInfo, 256, "%T", t);
                message.Status = INPUT;
                InsertMessage(message);

                if(packet.isEnd == false)
                {
                    memset(buffer, '\0', SIZE_MESSAGE_PACKET);
                    if( 0 > recv(ResultSocket, &buffer, SIZE_MESSAGE_PACKET, 0))
                    {
                        mvwprintw(Output, 1, 1, "%s%s\n", "Error Recv : ", strerror(errno));
                        wrefresh(Output);
                    }
                    else{
                        DeserializeMessage(buffer, &packet);
                        if(packet.header == Chunck)
                        {
                            strcpy(HeadMessage->message.Buf2, packet.Buffer);
                        }
                    }
                }
                else
                    strcpy(HeadMessage->message.Buf2, message.Buf2);
            }
            
            ShowMessage();
        }
    }
}

void SendNewClientMessage()
{
    struct MessagePacket packet;
    packet.header = Newclient;
    struct ifreq ifr;
    ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
    struct if_nameindex *name;
    name = if_nameindex();

    strncpy(ifr.ifr_ifrn.ifrn_name, if_indextoname(name->if_index + 1, name->if_name), IFNAMSIZ - 1);
    ioctl(ResultSocket, SIOCGIFADDR, &ifr);
    strcpy(packet.IP, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_ifru.ifru_addr)->sin_addr));
    strcpy(packet.Name, Name);
    packet.isEnd = true;
    packet.Size = strlen(packet.Buffer);
    
    char buffer[SIZE_MESSAGE_PACKET];
    SerializeMessage(buffer, packet);

    if(send(ResultSocket, buffer, SIZE_MESSAGE_PACKET, 0) < 0)
    {
        mvwprintw(Output, 1, 1, "%s%s\n", "Error Send Packet NewClient : ", strerror(errno));
        wrefresh(Output);
        sleep(5);
        CleanUP_UI();
        exit(0);
    }
}

char* GetIP()
{
    struct ifreq ifr;
    ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
    struct if_nameindex *name;
    name = if_nameindex();
    strncpy(ifr.ifr_ifrn.ifrn_name, if_indextoname(name->if_index + 1, name->if_name), IFNAMSIZ - 1);
    ioctl(ResultSocket, SIOCGIFADDR, &ifr);
    return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_ifru.ifru_addr)->sin_addr);
}

void InsertMessage(struct ClientMessage msg)
{
    struct ClientMessageList *node = (struct ClientMessageList *) malloc(sizeof(struct ClientMessageList));
    
    node->message.ID = msg.ID;
    strcpy(node->message.Buf1, msg.Buf1);
    strcpy(node->message.IP, msg.IP);
    strcpy(node->message.Name, msg.Name);
    strcpy(node->message.TimeInfo, msg.TimeInfo);
    node->message.Status = msg.Status;

    if(HeadMessage != NULL)
    if(HeadMessage->message.ID == 10)
    {
        delete(1);
        CurrentMessage = HeadMessage;
        while(CurrentMessage != NULL)
        {
            CurrentMessage->message.ID -= 1;
            CurrentMessage = CurrentMessage->next;
        }
        node->message.ID -= 1;
    }

    node->next = HeadMessage;
    HeadMessage = node;
}

//delete a link with given socketID
struct ClientMessageList* delete(int id) {

   //start from the first link
   struct ClientMessageList* current = HeadMessage;
   struct ClientMessageList* previous = NULL;
	
   //if list is empty
   if(HeadMessage == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->message.ID != id) {

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
   if(current == HeadMessage) {
      //change first to point to next link
      HeadMessage = HeadMessage->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}

void ShowMessage()
{
    CurrentMessage = HeadMessage;
    int i = 1;
    wclear(Output);
    draw_borders(Output);

    while (CurrentMessage != NULL)
    {
        mvwprintw(Output, i, 1, "%s [%s] ", CurrentMessage->message.IP, CurrentMessage->message.Name);
        if(CurrentMessage->message.Status == INPUT)
        {
            wprintw(Output, "<< %s ", CurrentMessage->message.Buf1);
            mvwprintw(Output, i, 69, "(%s)", CurrentMessage->message.TimeInfo);
        }
        else // OUTPUT
        {
            wprintw(Output, ">> %s ", CurrentMessage->message.Buf1);
            mvwprintw(Output, i, 69, "(%s)", CurrentMessage->message.TimeInfo);
        }

        if(strcmp(CurrentMessage->message.Buf2, "\0") != 0)
        {
            i += 1;
            if(CurrentMessage->message.Status == INPUT)
            {
                mvwprintw(Output, i, 1, "<< %s ", CurrentMessage->message.Buf2);
                mvwprintw(Output, i, 69, "(%s)", CurrentMessage->message.TimeInfo);
            }
            else // OUTPUT
            {
                mvwprintw(Output, i, 1, ">> %s ", CurrentMessage->message.Buf2);
                mvwprintw(Output, i, 69, "(%s)", CurrentMessage->message.TimeInfo);                 
            }
        }

        i += 1;
        CurrentMessage = CurrentMessage->next;
    }
    wrefresh(Output);
    wclear(Input);
    draw_borders(Input);
    mvwprintw(Input, 1, 1, "%s", "> ");
    wprintw(Input, "%s", Buf);
    wrefresh(Input);
}