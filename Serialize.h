/*  This static SerializeMessage() prepares the package for sending.
    This DeserializeMessage() prepares the package for reading.
*/


#ifndef SERIALIZE_H
#define SERIZLIZE_H

#include <stdio.h>
#include <string.h>
#include "Packet.h"

size_t SizeMessage(struct MessagePacket);

static size_t SerializeMessage(char *buffer, struct MessagePacket input)
{
    memcpy(buffer, &input, sizeof(input));
    return sizeof(input);
}

static size_t DeserializeMessage(char *buffer, struct MessagePacket *output)
{
    memcpy(output, buffer, sizeof(*output));
    return sizeof(*output);
}

#endif