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

#ifndef EPRO_TYPES_H
#define EPRO_TYPES_H

#include "config.h"

#include <stdbool.h>
#include <stdint.h>

#define ASCII_ACK  0x06
#define ASCII_NACK 0x15

typedef enum
{
    BITRATE_HINT_SLOW_REGULAR,
    BITRATE_HINT_SLOW_ABERRANT,
    BITRATE_HINT_FAST_REGULAR,
    BITRATE_HINT_FAST_ABERRANT,

    BITRATE_HINT_COUNT

} bitrate_hint_t;


typedef enum
{
    INTERFACE_RS232,
    INTERFACE_IRDA,
    INTERFACE_SPI,
    INTERFACE_I2C,

    INTERFACE_COUNT

} interface_t;


typedef enum
{
#if EPRO_BOARD_REVISION == 7
    KEY_DOWN  = 0x01,
    KEY_UP    = 0x02,
    KEY_BACK  = 0x04,
    KEY_OK    = 0x08,
#elif EPRO_BOARD_REVISION == 8
    KEY_UP    = 0x01,
    KEY_BACK  = 0x02,
    KEY_OK    = 0x04,
    KEY_DOWN  = 0x08,
#elif EPRO_BOARD_REVISION == 9
    KEY_UP    = 0x01,
    KEY_DOWN  = 0x02,
    KEY_OK    = 0x04,
    KEY_BACK  = 0x08,
#endif

    KEY_COUNT = 4

} key_t;


typedef enum
{
    RESULT_SUCCESS,
    RESULT_FAILED,
    RESULT_ABORTED,
    RESULT_TIMEOUT,
    RESULT_ERROR,

    RESULT_COUNT

} result_t;

#endif // EPRO_TYPES_H
