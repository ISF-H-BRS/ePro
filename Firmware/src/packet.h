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

#ifndef EPRO_PACKET_H
#define EPRO_PACKET_H

#include "config.h"

#include <stdint.h>

#define PACKET_MAGIC_NUMBER 0xfe

typedef struct
{
    const uint8_t magic_number;

    uint8_t index[3];
    uint8_t total[3];

    uint8_t data[EPRO_BLOCK_LENGTH];

    uint8_t checksum;

} __attribute__((packed)) packet_t;

void packet_init(packet_t *packet, uint8_t index, uint8_t total, const void *data);
void packet_copy(packet_t *dst, const packet_t *src);

uint8_t packet_get_index(const packet_t *packet);
uint8_t packet_get_total(const packet_t *packet);
uint8_t packet_compute_checksum(const packet_t *packet);

#endif // EPRO_PACKET_H
