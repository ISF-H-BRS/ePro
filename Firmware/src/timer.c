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

#include "timer.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#define NUM_TIMER_SLOTS 10

static timer_t *timers[NUM_TIMER_SLOTS];
static uint8_t num_timers = 0;

static void _timer_start_hw(void);
static void _timer_stop_hw(void);


ISR(TIMER2_COMP_vect)
{
    for (uint8_t i = 0; i < NUM_TIMER_SLOTS; ++i)
    {
        if (timers[i])
            ++timers[i]->msecs;
    }
}


bool timer_start(timer_t *timer)
{
    uint8_t free_slot = NUM_TIMER_SLOTS;
    for (uint8_t i = 0; i < NUM_TIMER_SLOTS; ++i)
    {
        if (timers[i] == timer)
            return true;
        else if (timers[i] == 0 && free_slot == NUM_TIMER_SLOTS)
            free_slot = i;
    }

    if (free_slot < NUM_TIMER_SLOTS)
    {
        timer->msecs = 0;
        timers[free_slot] = timer;

        ++num_timers;
        if (num_timers == 1)
            _timer_start_hw();

        return true;
    }

    return false;
}


void timer_stop(timer_t *timer)
{
    for (uint8_t i = 0; i < NUM_TIMER_SLOTS; ++i)
    {
        if (timers[i] == timer)
        {
            timers[i] = 0;

            --num_timers;
            if (num_timers == 0)
                _timer_stop_hw();

            return;
        }
    }
}


void _timer_start_hw()
{
    // Enable CTC mode
    TCCR2 = (1<<WGM21);

    // Prescale timer clock by 256
    TCCR2 |= (1<<CS22) | (1<<CS21);

    // Set compare value
    // F_CPU / 256 / 1000 = 31.25;
    OCR2 = 32;

    // Enable output compare match interrupt
    TIMSK |= (1<<OCIE2);
}


void _timer_stop_hw()
{
    TCCR2 = 0x00;
}
