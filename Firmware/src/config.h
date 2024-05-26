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

#ifndef EPRO_CONFIG_H
#define EPRO_CONFIG_H

#define EPRO_VERSION_STRING "7.1"

#define EPRO_BOARD_REVISION 9

#define EPRO_IRDA_SUPPORTED 1

#define EPRO_MESSAGE_COUNT 10

#define EPRO_BLOCK_LENGTH 8

#define EPRO_LCD_WIDTH 16

#define EPRO_DEFAULT_INTERFACE_INDEX 0

#define EPRO_DEFAULT_TEST_MESSAGE "ePro WS 2015/16"

#define EPRO_DEFAULT_MESSAGE_INDEX 0

#define EPRO_DEFAULT_MESSAGE_KEY { '0', '0', '0', '0', '0', '0', '0', '0' }

#define EPRO_MAX_PIN_ATTEMPTS 3

#define EPRO_DEVICE_LOCK_ENABLED false

#define EPRO_PIN_LENGTH 6

#define EPRO_DEFAULT_PIN { '1', '3', '4', '2', '3', '5' }

#define EPRO_DEFAULT_DEBUG false

#define EPRO_SCROLL_DELAY 300

#endif // EPRO_CONFIG_H
