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

#ifndef EPRO_H
#define EPRO_H

#include "menu.h"
#include "message.h"
#include "packet.h"
#include "types.h"

// Main functions
void epro_initialize(interface_t interface);
void epro_select_interface(interface_t interface);
void epro_set_bitrate_hint(bitrate_hint_t hint);
bool epro_get_input(const char *caption, uint8_t *input, uint8_t length, bool digits_only);

// Timing
void epro_delay_ms(uint16_t milliseconds);
result_t epro_wait_ms(uint16_t milliseconds);

// Menu
void epro_process_menu(menu_t *menu);

// Keys
void epro_poll_keys(void);
bool epro_is_key_pressed(key_t key);

// Packets
result_t epro_send_packet(const packet_t *packet);
result_t epro_read_packet(packet_t *packet);

// Messages
result_t epro_send_message(const message_t *message);
result_t epro_read_message(message_t *message);

#endif // EPRO_H
