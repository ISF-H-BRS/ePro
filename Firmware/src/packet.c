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

#include "packet.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

void packet_init(packet_t *packet, uint8_t index, uint8_t total, const void *data)
{
    char string[4];

    *(uint8_t*)(&packet->magic_number) = PACKET_MAGIC_NUMBER;

    memset(string, 0, sizeof(string));
    snprintf(string, 3, "%hhu", index);
    memcpy(packet->index, string, 3);

    memset(string, 0, sizeof(string));
    snprintf(string, 3, "%hhu", total);
    memcpy(packet->total, string, 3);

    memcpy(packet->data, data, EPRO_BLOCK_LENGTH);

    packet->checksum = packet_compute_checksum(packet);
}


void packet_copy(packet_t *dst, const packet_t *src)
{
    *(uint8_t*)(&dst->magic_number) = PACKET_MAGIC_NUMBER;

    memcpy(dst->index, src->index, 3);
    memcpy(dst->total, src->total, 3);
    memcpy(dst->data, src->data, EPRO_BLOCK_LENGTH);
    dst->checksum = src->checksum;
}


uint8_t packet_get_index(const packet_t *packet)
{
    return block_to_number(packet->index, 3);
}


uint8_t packet_get_total(const packet_t *packet)
{
    return block_to_number(packet->total, 3);
}


uint8_t packet_compute_checksum(const packet_t *packet)
{
    uint8_t checksum = 0;

    for(uint8_t i = 0; i < EPRO_BLOCK_LENGTH; ++i)
        checksum = ((uint16_t)checksum + packet->data[i]) % 256;

    if (checksum == PACKET_MAGIC_NUMBER)
        checksum++;

    return checksum;
}
