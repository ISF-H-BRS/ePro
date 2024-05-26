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

#include "messagetable.h"

#include <avr/pgmspace.h>

#include <stdlib.h>

#include "_message_table.inc"

uint16_t message_table_get_size(void)
{
    return _MESSAGE_TABLE_SIZE;
}


char* message_table_read_at(uint16_t index)
{
    if (index >= _MESSAGE_TABLE_SIZE)
        index = _MESSAGE_TABLE_SIZE - 1;

    const char *strptr = (const char *)pgm_read_word(&_message_table[index]);
    char *message = malloc(strlen_P(strptr) + 1);
    strcpy_P(message, strptr);
    return message;
}
