/*  The Packet code is implemented in this file.   */
/*  A enum defined for packet headers.          */

/*  enum Header
    {
        Message,    // This indicates the type of message packet.
        Chunck,     // This indicates that the packet is a continuation of the packet.
        Newclient,  // This indicates that the user information packet is new and should be saved.
        Bye         // This indicates to the server that the client wants to go offline.
    };                              */
/**/
/*  A data structure for packet information.    */
/*  struct MessagePacket
    {
        enum Header header;     // type packet.
        char Name[5];           // Name Client. 
        char IP[16];            // IP Client.
        bool isEnd;             // This indicates whether this package is the last package or not. (This does not mean that it is false).
        size_t Size;            // length Buffer.
        char Buffer[MAX_BUFFER]; //
    };                                                      */
/**/



#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdbool.h>

#define MAX_BUFFER 40
#define SIZE_ENUM 4
#define SIZE_MESSAGE_PACKET (MAX_BUFFER + SIZE_ENUM + 5 + 16 + 8 + 1 + 8)

enum Header
{
    Message,
    Chunck,
    Newclient,
    Bye
};


struct MessagePacket
{
    enum Header header;
    char Name[5];
    char IP[16];
    bool isEnd;
    size_t Size;
    char Buffer[MAX_BUFFER];
};


#endif