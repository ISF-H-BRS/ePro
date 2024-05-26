// ============================================================================================== //
//                                                                                                //
//  This file is part of the ePro firmware.                                                       //
//                                                                                                //
//  Author:                                                                                       //
//  Marcel Hasler <mahasler@gmail.com>                                                            //
//                                                                                                //
//  Copyright (c) 2010 - 2014                                                                     //
//  Bonn-Rhein-Sieg University of Applied Sciences                                                //
//                                                                                                //
//  This program is free software: you can redistribute it and/or modify it under the terms       //
//  of the GNU General Public License as published by the Free Software Foundation, either        //
//  version 3 of the License, or (at your option) any later version.                              //
//                                                                                                //
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;     //
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     //
//  See the GNU General Public License for more details.                                          //
//                                                                                                //
//  You should have received a copy of the GNU General Public License along with this program.    //
//  If not, see <https://www.gnu.org/licenses/>.                                                  //
//                                                                                                //
// ============================================================================================== //

#include "message.h"
#include "util.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Private functions
static uint16_t _message_get_block_count(const message_header_t *header);
static void _message_cipher(const uint8_t *block, const uint8_t *key, uint8_t *result);


void message_init(message_t *message, const char *string, const uint8_t *key)
{
    uint16_t message_len = strlen(string);
    uint16_t num_blocks = message_len / EPRO_BLOCK_LENGTH;
    if (message_len % EPRO_BLOCK_LENGTH > 0)
        num_blocks++;

    if (key)
        memcpy(message->header.key, key, EPRO_BLOCK_LENGTH);
    else
        memset(message->header.key, 0, EPRO_BLOCK_LENGTH);

    snprintf((char*)message->header.block_count, EPRO_BLOCK_LENGTH, "%d", num_blocks);

    message->blocks = (message_block_t*)malloc(num_blocks * sizeof(message_block_t));
    for (uint16_t i = 0; i < num_blocks; ++i)
    {
        // Cipher message block
        uint8_t result[EPRO_BLOCK_LENGTH];
        _message_cipher((uint8_t*)string + i*EPRO_BLOCK_LENGTH, message->header.key, result);

        memcpy(message->blocks[i].data, result, EPRO_BLOCK_LENGTH);
    }
}


void message_free(message_t *message)
{
    free(message->blocks);
}


char* message_get_string(const message_t *message)
{
    uint16_t block_count = _message_get_block_count(&message->header);

    char *string = (char*)malloc(block_count * EPRO_BLOCK_LENGTH);
    for (uint16_t i = 0; i < block_count; ++i)
    {
        // Decipher message block
        uint8_t result[EPRO_BLOCK_LENGTH];
        _message_cipher(message->blocks[i].data, message->header.key, result);

        memcpy(string + i*EPRO_BLOCK_LENGTH, result, EPRO_BLOCK_LENGTH);
    }

    return string;
}


void message_to_packets(const message_t *message, packet_t **packets, uint16_t *num_packets)
{
    uint16_t block_count = _message_get_block_count(&message->header);

    *num_packets = block_count + 2;
    *packets = (packet_t*)malloc(*num_packets * sizeof(packet_t));
    if (*packets == 0)
    {
        *num_packets = 0;
        return;
    }

    // Create packets for header
    uint8_t header_buffer[2*EPRO_BLOCK_LENGTH];
    memcpy(header_buffer, &message->header, sizeof(message_header_t));

    packet_init(&(*packets)[0], 1, *num_packets, &header_buffer[0]);
    packet_init(&(*packets)[1], 2, *num_packets, &header_buffer[EPRO_BLOCK_LENGTH]);

    // Create packets for message blocks
    for (uint16_t i = 0; i < block_count; ++i)
        packet_init(&(*packets)[i+2], i+3, *num_packets, message->blocks[i].data);
}


void message_from_packets(message_t *message, const packet_t *packets, uint16_t num_packets)
{
    // The message header takes two packets
    if (num_packets < 2)
        return;

    uint8_t header_buffer[2*EPRO_BLOCK_LENGTH];
    memcpy(&header_buffer[0*EPRO_BLOCK_LENGTH], packets[0].data, EPRO_BLOCK_LENGTH);
    memcpy(&header_buffer[1*EPRO_BLOCK_LENGTH], packets[1].data, EPRO_BLOCK_LENGTH);

    message_header_t header;
    memcpy(&header, header_buffer, sizeof(message_header_t));

    uint16_t block_count = _message_get_block_count(&header);
    if (block_count != (num_packets - 2))
        return;

    message->header = header;
    message->blocks = (message_block_t*)malloc(block_count * sizeof(message_block_t));
    for (uint16_t i = 0; i < block_count; ++i)
        memcpy(message->blocks[i].data, packets[i+2].data, EPRO_BLOCK_LENGTH);
}


uint16_t _message_get_block_count(const message_header_t *header)
{
    return block_to_number(header->block_count, EPRO_BLOCK_LENGTH);
}


void _message_cipher(const uint8_t *block, const uint8_t *key, uint8_t *result)
{
    for (uint8_t i = 0; i < EPRO_BLOCK_LENGTH; ++i)
        result[i] = block[i] ^ key[i];
}
