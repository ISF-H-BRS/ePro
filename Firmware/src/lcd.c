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

#include "lcd.h"
#include "support/lcd.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const uint8_t mode_commands[LCD_MODE_COUNT] =
{
    LCD_DISP_ON,
    LCD_DISP_ON_CURSOR,
    LCD_DISP_ON_BLINK,
    LCD_DISP_ON_CURSOR_BLINK
};


static void _lcd_print(uint8_t row, char *output);


void lcd_initialize()
{
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
}


void lcd_clear()
{
    lcd_clrscr();
}


void lcd_set_mode(lcd_mode_t mode)
{
    lcd_command(mode_commands[mode]);
}


void lcd_printf(uint8_t row, const char *format, ...)
{
    char output[LCD_WIDTH+1];

    // Build output string
    va_list args;
    va_start(args, format);
    vsnprintf(output, LCD_WIDTH+1, format, args);
    va_end(args);

    _lcd_print(row, output);
}


void lcd_printf_P(uint8_t row, const char *format, ...)
{
    char output[LCD_WIDTH+1];

    // Build output string
    va_list args;
    va_start(args, format);
    vsnprintf_P(output, LCD_WIDTH+1, format, args);
    va_end(args);

    _lcd_print(row, output);
}


void lcd_set_cursor_position(uint8_t row, uint8_t pos)
{
    if (row > 1)
        row = 1;

    if (pos >= LCD_WIDTH)
        pos = LCD_WIDTH - 1;

    lcd_gotoxy(pos, row);
}


void _lcd_print(uint8_t row, char *output)
{
    if (row > 1)
        row = 1;

    // Fill up with empty space
    output[LCD_WIDTH] = '\0';
    size_t length = strlen(output);
    if (length < LCD_WIDTH)
        memset(output+length, ' ', LCD_WIDTH-length);

    lcd_gotoxy(0, row);
    lcd_puts(output);
}
