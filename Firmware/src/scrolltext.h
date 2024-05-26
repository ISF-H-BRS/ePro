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

#ifndef EPRO_SCROLLTEXT_H
#define EPRO_SCROLLTEXT_H

#include <stdint.h>

typedef struct
{
    char *string;
    uint8_t length;

    char *text;
    uint8_t position;

} scrolltext_t;


void scrolltext_init(scrolltext_t *scrolltext, const char *text, uint8_t length, uint8_t gap);
void scrolltext_free(scrolltext_t *scrolltext);

void scrolltext_update(scrolltext_t *scrolltext);
const char* scrolltext_read(const scrolltext_t *scrolltext);

#endif // EPRO_SCROLLTEXT_H
