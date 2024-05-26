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

#ifndef EPRO_UART_H
#define EPRO_UART_H

#include "interface.h"
#include "packet.h"

#include <stdbool.h>
#include <stdint.h>

// These functions are intended for internal
// use by the RS-232 and IrDA modules only.

extern interface_status_t _uart_status;

void _uart_initialize(void);
void _uart_set_baudrate(uint32_t baudrate);
void _uart_set_ir_enabled(bool enable);
void _uart_shutdown(void);
void _uart_send_byte(uint8_t byte);
void _uart_send_packet(const packet_t *packet);
void _uart_read_packet(packet_t *packet);
void _uart_abort(void);

#endif // EPRO_UART_H
