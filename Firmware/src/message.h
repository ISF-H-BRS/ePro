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

#ifndef EPRO_MESSAGE_H
#define EPRO_MESSAGE_H

#include "config.h"
#include "packet.h"

typedef struct
{
    uint8_t block_count[EPRO_BLOCK_LENGTH];
    uint8_t key[EPRO_BLOCK_LENGTH];

} message_header_t;

typedef struct
{
    uint8_t data[EPRO_BLOCK_LENGTH];

} message_block_t;

typedef struct
{
    message_header_t header;
    message_block_t *blocks;

} message_t;

void message_init(message_t *message, const char *string, const uint8_t *key);
void message_free(message_t *message);

void message_to_packets(const message_t *message, packet_t **packets, uint16_t *num_packets);
void message_from_packets(message_t *message, const packet_t *packets, uint16_t num_packets);

char* message_get_string(const message_t *message);

#endif // EPRO_MESSAGE_H
