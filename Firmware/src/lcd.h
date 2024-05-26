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

#ifndef EPRO_LCD_H
#define EPRO_LCD_H

#include <avr/pgmspace.h>

#include <stdbool.h>
#include <stdint.h>

#define LCD_WIDTH 16

typedef enum
{
    LCD_MODE_CURSOR_OFF_BLINK_OFF,
    LCD_MODE_CURSOR_ON_BLINK_OFF,
    LCD_MODE_CURSOR_OFF_BLINK_ON,
    LCD_MODE_CURSOR_ON_BLINK_ON,

    LCD_MODE_COUNT

} lcd_mode_t;

void lcd_initialize(void);
void lcd_clear(void);
void lcd_set_mode(lcd_mode_t mode);

void lcd_printf(uint8_t row, const char *format, ...);
void lcd_printf_P(uint8_t row, const char *format, ...);

#define lcd_printf_PSTR(row, format, ...) lcd_printf_P(row, PSTR(format), ##__VA_ARGS__)

void lcd_set_cursor_position(uint8_t row, uint8_t pos);

#endif // EPRO_LCD_H
