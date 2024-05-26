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

#ifndef EPRO_MENU_H
#define EPRO_MENU_H

#include <stdbool.h>
#include <stdint.h>

#define MENU_INIT(_name, _caption, _entry_count, _entries, _auto_quit)  \
static menu_t _name =                                                   \
{                                                                       \
    .caption       = _caption,                                          \
    .entry_count   = _entry_count,                                      \
    .entries       = _entries,                                          \
    .auto_quit     = _auto_quit,                                        \
    .current_entry = 0                                                  \
};                                                                      \

typedef void (*menu_callback_t)(void);

typedef struct
{
    const char *text;
    menu_callback_t callback;

} menu_entry_t;

typedef struct
{
    const char *caption;
    const uint8_t entry_count;
    const menu_entry_t *entries;
    bool auto_quit;
    uint8_t current_entry;

} menu_t;

#endif // EPRO_MENU_H
