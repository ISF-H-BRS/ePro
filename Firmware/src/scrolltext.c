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

#include "scrolltext.h"

#include <stdlib.h>
#include <string.h>

static void _scrolltext_build_string(scrolltext_t *scrolltext);


void scrolltext_init(scrolltext_t *scrolltext, const char *text, uint8_t length, uint8_t gap)
{
    uint8_t text_length = strlen(text) + gap;

    scrolltext->string = malloc(length+1);
    scrolltext->length = length;

    scrolltext->text = malloc(text_length+1);
    scrolltext->position = 0;

    memset(scrolltext->text, ' ', text_length);
    memcpy(scrolltext->text, text, strlen(text));
    scrolltext->text[text_length] = '\0';

    _scrolltext_build_string(scrolltext);
}


void scrolltext_free(scrolltext_t *scrolltext)
{
    free(scrolltext->string);
    free(scrolltext->text);
}


void scrolltext_update(scrolltext_t *scrolltext)
{
    scrolltext->position = (scrolltext->position + 1) % strlen(scrolltext->text);
    _scrolltext_build_string(scrolltext);
}


const char* scrolltext_read(const scrolltext_t *scrolltext)
{
    return scrolltext->string;
}


void _scrolltext_build_string(scrolltext_t *scrolltext)
{
    uint8_t position = scrolltext->position;
    for (uint8_t i = 0; i < scrolltext->length; ++i)
    {
        scrolltext->string[i] = scrolltext->text[position++];
        if (position >= strlen(scrolltext->text))
            position = 0;
    }

    scrolltext->string[scrolltext->length] = '\0';
}
