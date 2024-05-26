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

#include "rs232.h"
#include "uart.h"

static const uint32_t bitrates[BITRATE_HINT_COUNT] =
{
    9600,   // BITRATE_HINT_SLOW_STANDARD
    12345,  // BITRATE_HINT_SLOW_ABERRANT
    115200, // BITRATE_HINT_FAST_STANDARD
    123456, // BITRATE_HINT_FAST_ABERRANT
};

static void _rs232_initialize(bitrate_hint_t hint);


void rs232_alloc_interface(interface_driver_t *interface)
{
    interface->initialize_tx = _rs232_initialize;
    interface->initialize_rx = _rs232_initialize;
    interface->shutdown      = _uart_shutdown;

    interface->send_packet   = _uart_send_packet;
    interface->read_packet   = _uart_read_packet;
    interface->abort         = _uart_abort;

    interface->status        = &_uart_status;
}


void _rs232_initialize(bitrate_hint_t hint)
{
    _uart_initialize();
    _uart_set_baudrate(bitrates[hint]);
}
