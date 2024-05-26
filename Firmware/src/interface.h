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

#ifndef EPRO_INTERFACE_H
#define EPRO_INTERFACE_H

#include "packet.h"
#include "types.h"

typedef struct
{
    volatile result_t result;
    volatile bool done;

} interface_status_t;


typedef struct
{
    void (*initialize_tx)(bitrate_hint_t hint);
    void (*initialize_rx)(bitrate_hint_t hint);
    void (*shutdown)(void);

    void (*send_packet)(const packet_t *packet);
    void (*read_packet)(packet_t *packet);
    void (*abort)(void);

    interface_status_t *status;

} interface_driver_t;

#endif // EPRO_INTERFACE_H
