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

#ifndef EPRO_SETTINGS_H
#define EPRO_SETTINGS_H

#include "config.h"

#include <stdint.h>

typedef struct
{
    uint8_t interface_index;
    uint16_t message_index;
    uint8_t test_message[EPRO_LCD_WIDTH];
    uint8_t message_key[EPRO_BLOCK_LENGTH];
    bool debug;

#if EPRO_DEVICE_LOCK_ENABLED
    bool locked;
#endif

} settings_t;

void settings_save(const settings_t *settings);
void settings_load(settings_t *settings);

void settings_process_menu(settings_t *settings);

#endif // EPRO_SETTINGS_H
