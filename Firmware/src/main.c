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

#include "config.h"
#include "epro.h"
#include "lcd.h"
#include "messagetable.h"
#include "settings.h"

#include <stdlib.h>
#include <string.h>


// Forward declarations
static void send(const message_t *message);
static void read(void);


// Settings
static settings_t current_settings;


// Result strings
static const char result_string0[] PROGMEM = "Success!";
static const char result_string1[] PROGMEM = "Failed!";
static const char result_string2[] PROGMEM = "Aborted!";
static const char result_string3[] PROGMEM = "Timeout!";
static const char result_string4[] PROGMEM = "Error!";
static PGM_P const result_strings[RESULT_COUNT] =
{
    result_string0,
    result_string1,
    result_string2,
    result_string3,
    result_string4
};


// Main menu
static void send_test(void);
static void read_test(void);

static void send_message(void);
static void read_message(void);

static void show_settings(void);

static const menu_entry_t main_menu_entries[] =
{
    { "Send test",    send_test     },
    { "Read test",    read_test     },
    { "Send message", send_message  },
    { "Read message", read_message  },
    { "Settings",     show_settings }
};

MENU_INIT(main_menu, "Main menu:", 5, main_menu_entries, false);


// Function definitions
void send(const message_t *message)
{
    for (uint8_t i = 0; i < EPRO_MESSAGE_COUNT; ++i)
    {
        lcd_clear();
        lcd_printf_PSTR(0, "Sending %02d/%02d...", i+1, EPRO_MESSAGE_COUNT);

        result_t result = epro_send_message(message);

        lcd_printf_P(1, result_strings[result]);
        if (result == RESULT_ABORTED || epro_wait_ms(1000) == RESULT_ABORTED)
        {
            lcd_printf_P(1, result_strings[RESULT_ABORTED]);
            epro_delay_ms(1000);
            return;
        }
    }
}


void read()
{
    while (1)
    {
        lcd_clear();
        lcd_printf_PSTR(0, "Waiting...");

        message_t message;
        result_t result = epro_read_message(&message);

        if (current_settings.debug && result == RESULT_SUCCESS)
        {
            char* msg = message_get_string(&message);

            lcd_printf_PSTR(0, "Message:");
            lcd_printf_PSTR(1, "%s", msg);

            free(msg);
        }
        else
            lcd_printf_P(1, result_strings[result]);

        if (result == RESULT_SUCCESS)
            message_free(&message);

        if (result == RESULT_ABORTED || epro_wait_ms(800) == RESULT_ABORTED)
        {
            lcd_printf_P(1, result_strings[RESULT_ABORTED]);
            epro_delay_ms(1000);
            return;
        }
    }
}


void send_test()
{
    epro_set_bitrate_hint(BITRATE_HINT_SLOW_ABERRANT);

    char *message = malloc(EPRO_LCD_WIDTH + 1);
    memcpy(message, current_settings.test_message, EPRO_LCD_WIDTH);
    message[EPRO_LCD_WIDTH] = '\0';

    message_t msg;
    message_init(&msg, message, 0);
    free(message);
    send(&msg);
    message_free(&msg);
}


void read_test()
{
    epro_set_bitrate_hint(BITRATE_HINT_SLOW_ABERRANT);
    read();
}


void send_message()
{
    epro_set_bitrate_hint(BITRATE_HINT_SLOW_REGULAR);

    char *message = message_table_read_at(current_settings.message_index);

    message_t msg;
    message_init(&msg, message, current_settings.message_key);
    free(message);
    send(&msg);
    message_free(&msg);
}


void read_message()
{
    epro_set_bitrate_hint(BITRATE_HINT_SLOW_REGULAR);
    read();
}


void show_settings()
{
    settings_process_menu(&current_settings);
}


int main(void)
{
    // Load settings from EEPROM & initialize ePro library
    settings_load(&current_settings);
    epro_initialize(current_settings.interface_index);

    // Print info
    lcd_printf_PSTR(0, "ePro Firmware");
    lcd_printf_PSTR(1, "Version %s", EPRO_VERSION_STRING);
    epro_delay_ms(2000);

#if EPRO_DEVICE_LOCK_ENABLED
    if (current_settings.locked)
    {
        lcd_clear();
        lcd_printf_PSTR(0, "Device locked!");
        while (1);
    }
#endif

    // Start processing
    while (1)
        epro_process_menu(&main_menu);

    return 0;
}
