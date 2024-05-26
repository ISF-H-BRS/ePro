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

#include "epro.h"
#include "lcd.h"
#include "messagetable.h"
#include "scrolltext.h"
#include "settings.h"
#include "timer.h"

#include <avr/eeprom.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


// EEPROM addresses
static uint8_t eep_interface_index EEMEM = EPRO_DEFAULT_INTERFACE_INDEX;
static uint8_t eep_test_message[EPRO_LCD_WIDTH] EEMEM = EPRO_DEFAULT_TEST_MESSAGE;
static uint16_t eep_message_index EEMEM = EPRO_DEFAULT_MESSAGE_INDEX;
static uint8_t eep_message_key[EPRO_BLOCK_LENGTH] EEMEM = EPRO_DEFAULT_MESSAGE_KEY;
static uint8_t eep_debug EEMEM = EPRO_DEFAULT_DEBUG ? 1 : 0;
static uint8_t eep_pin[EPRO_PIN_LENGTH] EEMEM = EPRO_DEFAULT_PIN;

#if EPRO_DEVICE_LOCK_ENABLED
static uint8_t eep_locked EEMEM = 0;
#endif


// Write/read single values to/from EEPROM
static void _settings_write_interface_index(uint8_t index);
static void _settings_read_interface_index(uint8_t *index);

static void _settings_write_message_index(uint16_t index);
static void _settings_read_message_index(uint16_t *index);

static void _settings_write_test_message(const uint8_t *message);
static void _settings_read_test_message(uint8_t *message);

static void _settings_write_message_key(const uint8_t *key);
static void _settings_read_message_key(uint8_t *key);

static void _settings_write_debug(bool debug);
static void _settings_read_debug(bool *debug);

static void _settings_write_pin(const uint8_t *pin);
static void _settings_read_pin(uint8_t *pin);

#if EPRO_DEVICE_LOCK_ENABLED
static void _settings_write_locked(bool locked);
static void _settings_read_locked(bool *locked);
#endif


// Helper
static bool _settings_check_pin(void);


// Interface menu
static void _settings_select_rs232(void);
static void _settings_select_irda(void);
static void _settings_select_spi(void);
static void _settings_select_i2c(void);

static const menu_entry_t interface_menu_entries[] =
{
    { "RS-232", _settings_select_rs232 },
    { "IrDA",   _settings_select_irda  },
    { "SPI",    _settings_select_spi   },
    { "I2C",    _settings_select_i2c   }
};

MENU_INIT(interface_menu, "Select iface:", 4, interface_menu_entries, true);


// Admin menu
static void _settings_select_message(void);
static void _settings_set_test_message(void);
static void _settings_set_key(void);
static void _settings_toggle_debug(void);
static void _settings_change_pin(void);

static const menu_entry_t admin_menu_entries[] =
{
    { "Select message", _settings_select_message   },
    { "Set test msg",   _settings_set_test_message },
    { "Set key",        _settings_set_key          },
    { "Toggle debug",   _settings_toggle_debug     },
    { "Change PIN",     _settings_change_pin       }
};

MENU_INIT(admin_menu, "Administration:", 5, admin_menu_entries, false);


// Settings menu
static void _settings_select_interface(void);
static void _settings_show_admin_menu(void);

static const menu_entry_t settings_menu_entries[] =
{
    { "Select iface",   _settings_select_interface },
    { "Administration", _settings_show_admin_menu  }
};

MENU_INIT(settings_menu, "Settings:", 2, settings_menu_entries, false);


void settings_save(const settings_t *settings)
{
    _settings_write_interface_index(settings->interface_index);
    _settings_write_message_index(settings->message_index);
    _settings_write_test_message(settings->test_message);
    _settings_write_message_key(settings->message_key);
    _settings_write_debug(settings->debug);

#if EPRO_DEVICE_LOCK_ENABLED
    _settings_write_locked(settings->locked);
#endif
}


void settings_load(settings_t *settings)
{
    _settings_read_interface_index(&settings->interface_index);
    _settings_read_message_index(&settings->message_index);
    _settings_read_test_message(settings->test_message);
    _settings_read_message_key(settings->message_key);
    _settings_read_debug(&settings->debug);

#if EPRO_DEVICE_LOCK_ENABLED
    _settings_read_locked(&settings->locked);
#endif
}


void settings_process_menu(settings_t *settings)
{
    epro_process_menu(&settings_menu);
    if (settings)
        settings_load(settings);
}


void _settings_write_interface_index(uint8_t index)
{
    eeprom_write_byte(&eep_interface_index, index);
}


void _settings_read_interface_index(uint8_t *index)
{
    *index = eeprom_read_byte(&eep_interface_index);
    if (*index == 0xff)
        *index = 0;
}


void _settings_write_message_index(uint16_t index)
{
    eeprom_write_word(&eep_message_index, index);
}


void _settings_read_message_index(uint16_t *index)
{
    *index = eeprom_read_word(&eep_message_index);
    if (*index == 0xffff)
        *index = 0;
}


void _settings_write_test_message(const uint8_t *message)
{
    eeprom_write_block(message, eep_test_message, EPRO_LCD_WIDTH);
}


void _settings_read_test_message(uint8_t *message)
{
    eeprom_read_block(message, eep_test_message, EPRO_LCD_WIDTH);
}


void _settings_write_message_key(const uint8_t *key)
{
    eeprom_write_block(key, eep_message_key, EPRO_BLOCK_LENGTH);
}


void _settings_read_message_key(uint8_t *key)
{
    eeprom_read_block(key, eep_message_key, EPRO_BLOCK_LENGTH);

    bool key_blank = true;
    for (uint8_t i = 0; i < EPRO_BLOCK_LENGTH; ++i)
    {
        if (key[i] != 0xff)
        {
            key_blank = false;
            break;
        }
    }

    if (key_blank)
        memset(key, 0, EPRO_BLOCK_LENGTH);
}


void _settings_write_debug(bool debug)
{
    eeprom_write_byte(&eep_debug, (debug ? 1 : 0));
}


void _settings_read_debug(bool *debug)
{
    uint8_t db = eeprom_read_byte(&eep_debug);
    *debug = (db == 0xff) ? false : (db != 0);
}


void _settings_write_pin(const uint8_t *pin)
{
    eeprom_write_block(pin, eep_pin, EPRO_PIN_LENGTH);
}


void _settings_read_pin(uint8_t *pin)
{
    eeprom_read_block(pin, eep_pin, EPRO_PIN_LENGTH);

    bool pin_blank = true;
    for (uint8_t i = 0; i < EPRO_PIN_LENGTH; ++i)
    {
        if (pin[i] != 0xff)
        {
            pin_blank = false;
            break;
        }
    }

    if (pin_blank)
        memset(pin, '1', EPRO_PIN_LENGTH);
}


#if EPRO_DEVICE_LOCK_ENABLED
void _settings_write_locked(bool locked)
{
    eeprom_write_byte(&eep_locked, (locked ? 1 : 0));
}


void _settings_read_locked(bool *locked)
{
    uint8_t l = eeprom_read_byte(&eep_locked);
    *locked = (l == 0xff) ? false : (l != 0);
}
#endif


bool _settings_check_pin()
{
    uint8_t input[EPRO_PIN_LENGTH];
    memset(input, '0', EPRO_PIN_LENGTH);
    if (!epro_get_input("Enter PIN:", input, EPRO_PIN_LENGTH, true))
        return false;

    uint8_t pin[EPRO_PIN_LENGTH];
    _settings_read_pin(pin);

    bool result = true;
    for (uint8_t i = 0; i < EPRO_PIN_LENGTH; ++i)
    {
        if (input[i] != pin[i])
        {
            result = false;
            break;
        }
    }

    lcd_clear();
    if (result)
    {
        lcd_printf_PSTR(0, "Correct!");
        epro_delay_ms(1000);
    }
    else
    {
        lcd_printf_PSTR(0, "Wrong PIN!");
        epro_delay_ms(5000);
    }

    return result;
}


void _settings_select_rs232()
{
    epro_select_interface(INTERFACE_RS232);
    _settings_write_interface_index(INTERFACE_RS232);

    lcd_clear();
    lcd_printf_PSTR(0, "Using RS-232.");
    epro_delay_ms(1000);
}


void _settings_select_irda()
{
#if EPRO_IRDA_SUPPORTED && EPRO_BOARD_REVISION >= 8
    epro_select_interface(INTERFACE_IRDA);
    _settings_write_interface_index(INTERFACE_IRDA);

    lcd_clear();
    lcd_printf_PSTR(0, "Using IrDA.");
    epro_delay_ms(1000);
#else
    lcd_printf_PSTR(0, "Board doesn't");
    lcd_printf_PSTR(1, "support IrDA!");
    epro_delay_ms(2000);
#endif
}


void _settings_select_spi()
{
    epro_select_interface(INTERFACE_SPI);
    _settings_write_interface_index(INTERFACE_SPI);

    lcd_clear();
    lcd_printf_PSTR(0, "Using SPI.");
    epro_delay_ms(1000);
}


void _settings_select_i2c()
{
    epro_select_interface(INTERFACE_I2C);
    _settings_write_interface_index(INTERFACE_I2C);

    lcd_clear();
    lcd_printf_PSTR(0, "Using I2C.");
    epro_delay_ms(1000);

}


void _settings_select_message()
{
    uint16_t num_messages = message_table_get_size();

    uint16_t current_index = 0;
    _settings_read_message_index(&current_index);
    if (current_index >= num_messages)
        current_index = num_messages - 1;

    char *current_message = message_table_read_at(current_index);

    scrolltext_t scrolltext;
    scrolltext_init(&scrolltext, current_message, 16, 5);

    timer_t timer;
    timer_start(&timer);

    lcd_printf_PSTR(0, "Select message:");
    lcd_printf(1, scrolltext_read(&scrolltext));

    uint8_t delay_count = 0;

    bool needs_refresh = false;
    while (1)
    {
        epro_poll_keys();

        if (epro_is_key_pressed(KEY_DOWN))
        {
            current_index = (current_index + 1) % num_messages;
            needs_refresh = true;
        }
        
        else if (epro_is_key_pressed(KEY_UP))
        {
            current_index = (current_index + num_messages - 1) % num_messages;
            needs_refresh = true;
        }

        else if (epro_is_key_pressed(KEY_OK))
        {
            _settings_write_message_index(current_index);

            lcd_clear();
            lcd_printf_PSTR(0, "Message set.");
            epro_delay_ms(1000);

            goto done;
        }

        else if (epro_is_key_pressed(KEY_BACK))
            goto done;

        if (needs_refresh)
        {
            timer_stop(&timer);
            scrolltext_free(&scrolltext);
            free(current_message);

            current_message = message_table_read_at(current_index);
            scrolltext_init(&scrolltext, current_message, 16, 5);
            lcd_printf(1, scrolltext_read(&scrolltext));

            delay_count = 0;
            timer.msecs = 0;
            timer_start(&timer);

            needs_refresh = false;
        }

        if (timer.msecs >= EPRO_SCROLL_DELAY)
        {
            timer.msecs = 0;

            // Wait a little before scrolling
            if (delay_count >= 3)
            {
                scrolltext_update(&scrolltext);
                lcd_printf(1, scrolltext_read(&scrolltext));
            }
            else
                ++delay_count;
        }
    }

done:
    timer_stop(&timer);
    scrolltext_free(&scrolltext);
    free(current_message);
}


void _settings_set_test_message()
{
    uint8_t input[EPRO_LCD_WIDTH];
    _settings_read_test_message(input);
    if (!epro_get_input("Set new message:", input, EPRO_LCD_WIDTH, false))
        return;

    // Zero out trailing white space
    for (int8_t i = EPRO_LCD_WIDTH - 1; i >= 0; --i)
    {
        if (input[i] == ' ')
            input[i] = '\0';
        else
            break;
    }

    _settings_write_test_message(input);

    lcd_clear();
    lcd_printf_PSTR(0, "Message set.");
    epro_delay_ms(1000);
}


void _settings_set_key()
{
    uint8_t input[EPRO_BLOCK_LENGTH];
    _settings_read_message_key(input);
    if (!epro_get_input("Set new key:", input, EPRO_BLOCK_LENGTH, true))
        return;

    _settings_write_message_key(input);

    lcd_clear();
    lcd_printf_PSTR(0, "Key set.");
    epro_delay_ms(1000);
}


void _settings_toggle_debug()
{
    bool debug;
    _settings_read_debug(&debug);

    debug = !debug;
    _settings_write_debug(debug);

    lcd_clear();
    if (debug)
        lcd_printf_PSTR(0, "Debug enabled.");
    else
        lcd_printf_PSTR(0, "Debug disabled.");

    epro_delay_ms(1000);
}


void _settings_change_pin()
{
    uint8_t pin1[EPRO_PIN_LENGTH];
    memset(pin1, '0', EPRO_PIN_LENGTH);
    if (!epro_get_input("Enter new PIN:", pin1, EPRO_PIN_LENGTH, true))
        return;

    uint8_t pin2[EPRO_PIN_LENGTH];
    memset(pin2, '0', EPRO_PIN_LENGTH);
    if (!epro_get_input("Enter PIN again:", pin2, EPRO_PIN_LENGTH, true))
        return;

    for (uint8_t i = 0; i < EPRO_PIN_LENGTH; ++i)
    {
        if (pin1[i] != pin2[i])
        {
            lcd_clear();
            lcd_printf_PSTR(0, "No match! PIN");
            lcd_printf_PSTR(1, "was NOT saved.");
            epro_delay_ms(4000);
            return;
        }
    }

    _settings_write_pin(pin1);

    lcd_clear();
    lcd_printf_PSTR(0, "PIN saved.");
    epro_delay_ms(1000);
}


void _settings_select_interface()
{
    _settings_read_interface_index(&interface_menu.current_entry);
    if (interface_menu.current_entry >= interface_menu.entry_count)
        interface_menu.current_entry = 0;

    epro_process_menu(&interface_menu);
}


void _settings_show_admin_menu()
{
#if EPRO_MAX_PIN_ATTEMPTS > 0
    static uint8_t pin_attempts = 0;
#endif

    bool pin_correct = _settings_check_pin();
    if (pin_correct)
    {
        epro_process_menu(&admin_menu);

#if EPRO_MAX_PIN_ATTEMPTS > 0
        pin_attempts = 0;
    }
    else if (++pin_attempts >= 3)
    {
#if EPRO_DEVICE_LOCK_ENABLED
        _settings_write_locked(true);
#endif

        lcd_clear();
        lcd_printf_PSTR(0, "Device locked!");
        while (1);
#endif
    }
}
