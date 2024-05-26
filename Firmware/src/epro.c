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
#include "i2c.h"
#include "irda.h"
#include "lcd.h"
#include "rs232.h"
#include "spi.h"
#include "timer.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>

#include <stdlib.h>
#include <string.h>


#if defined(EPRO_BOARD_REVISION)
#if EPRO_BOARD_REVISION < 7 || EPRO_BOARD_REVISION > 9
#error Unknown EPRO_BOARD_REVISION defined!
#endif
#else
#error EPRO_BOARD_REVISION not defined, please define it in config.h!
#endif // EPRO_BOARD_REVISION


// Printable ASCII characters & digits
// Note: The HD44780 doesn't use a real ASCII table,
//       tilde is replaced by right arrow, backslash
//       by Yen symbol, 0x7f (DEL) is left arrow
#define CHARACTER_TABLE_SIZE 95
static const uint8_t character_table[CHARACTER_TABLE_SIZE] =
{
    ' ',

    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',

    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

    '.', ',', ':', ';', '!', '?', '&', '%', '#', '$', '@',
    '"', '\'', '`', '^', '+', '-', '*', '=', '<', '>',
    '_', '/', '|', '(', ')', '[', ']', '{', '}',
    0x7e, 0x7f // right & left arrow
};

#define DIGIT_TABLE_SIZE 10
static const uint8_t digit_table[DIGIT_TABLE_SIZE] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };


// The actual (possibly unstable) key state
static volatile uint8_t volatile_key_state = 0x00;

// The current and last stable key states
static uint8_t current_key_state = 0x00;
static uint8_t old_key_state = 0x00;

// The interface currently being used
static interface_driver_t current_interface;
static bitrate_hint_t current_bitrate_hint = BITRATE_HINT_SLOW_REGULAR;

// Forward declarations (private functions)
static void _epro_initialize_lcd(void);
static void _epro_initialize_keys(void);

static result_t _epro_send_packet(const packet_t *packet);
static result_t _epro_read_packet(packet_t *packet);


// Interrupt handler for key timer, used to debounce key presses. The state
// of the keys is considered stable if it hasn't changed for 10 ms.
#if EPRO_BOARD_REVISION < 9
ISR(TIMER0_COMP_vect)
{
    static volatile uint8_t old_state = 0x00;
    static volatile uint8_t new_state = 0x00;
    static volatile uint8_t count = 0;

    // Read key state (bit == 0 in PINA means key down, therefore invert)
    new_state = ~PINA;

    if (new_state == old_state)
        count++;
    else
        count = 0;

    old_state = new_state;

    if (count > 10)
    {
        // The key state should be stable now
        volatile_key_state = new_state;
        count = 0;
    }
}
#endif // EPRO_BOARD_REVISION < 9


void epro_initialize(interface_t interface)
{
    _epro_initialize_keys();
    _epro_initialize_lcd();

    epro_select_interface(interface);

    // Enable global interrupts
    sei();
}


void epro_select_interface(interface_t interface)
{
    switch (interface)
    {
        case INTERFACE_RS232:
            rs232_alloc_interface(&current_interface);
            break;

        case INTERFACE_IRDA:
            irda_alloc_interface(&current_interface);
            break;

        case INTERFACE_SPI:
            spi_alloc_interface(&current_interface);
            break;

        case INTERFACE_I2C:
            i2c_alloc_interface(&current_interface);
            break;

        default:
            break;
    }
}


void epro_set_bitrate_hint(bitrate_hint_t hint)
{
    current_bitrate_hint = hint;
}


bool epro_get_input(const char *caption, uint8_t *input, uint8_t length, bool digits_only)
{
    lcd_printf(0, caption);
    lcd_set_mode(LCD_MODE_CURSOR_ON_BLINK_OFF);

    if (length > 16)
        length = 16;

    const uint8_t *alphabet = digits_only ? digit_table : character_table;
    uint8_t num_letters = digits_only ? DIGIT_TABLE_SIZE : CHARACTER_TABLE_SIZE;
    uint8_t position = 0;

    char string[length+1];
    memcpy(string, input, length);
    string[length] = '\0';

    uint8_t indices[length];
    memset(indices, 0, length);

    for (uint8_t i = 0; i < length; ++i)
    {
        for (uint8_t j = 0; j < num_letters; ++j)
        {
            if (input[i] == alphabet[j])
            {
                indices[i] = j;
                break;
            }
        }
    }

    bool needs_refresh = true;
    while (1)
    {
        epro_poll_keys();

        if (epro_is_key_pressed(KEY_UP))
        {
            indices[position] = (indices[position] + 1) % num_letters;
            needs_refresh = true;
        }
        
        else if (epro_is_key_pressed(KEY_DOWN))
        {
            indices[position] = (indices[position] + num_letters - 1) % num_letters;
            needs_refresh = true;
        }

        else if (epro_is_key_pressed(KEY_OK))
        {
            if (position == (length-1))
                break;

            ++position;
            needs_refresh = true;
        }

        else if (epro_is_key_pressed(KEY_BACK))
        {
            if (position == 0)
                return false;

            --position;
            needs_refresh = true;
        }

        if (needs_refresh)
        {
            string[position] = alphabet[indices[position]];
            lcd_printf(1, "%s", string);
            lcd_set_cursor_position(1, position);
            needs_refresh = false;
        }
    }

    lcd_set_mode(LCD_MODE_CURSOR_OFF_BLINK_OFF);

    memcpy(input, string, length);
    return true;
}


void epro_delay_ms(uint16_t milliseconds)
{
    while (milliseconds-- > 0)
        _delay_ms(1);
}


result_t epro_wait_ms(uint16_t milliseconds)
{
    result_t result = RESULT_TIMEOUT;

    timer_t timer;
    timer_start(&timer);

    while (timer.msecs < milliseconds)
    {
        epro_poll_keys();
        if (epro_is_key_pressed(KEY_BACK))
        {
            result = RESULT_ABORTED;
            break;
        }
    }

    timer_stop(&timer);

    return result;
}


void epro_process_menu(menu_t *menu)
{
    if (menu->entry_count < 1)
    {
        lcd_clear();
        lcd_printf_PSTR(0, "No menu entries!");
        
        while (1)
        {
            epro_poll_keys();
            if (epro_is_key_pressed(KEY_BACK))
                return;
        }
    }

    if (menu->current_entry >= menu->entry_count)
        menu->current_entry = 0;

    bool needs_refresh = true;
    while (1)
    {
        epro_poll_keys();

        if (epro_is_key_pressed(KEY_DOWN))
        {
            menu->current_entry = (menu->current_entry + 1) % menu->entry_count;
            needs_refresh = true;
        }

        else if (epro_is_key_pressed(KEY_UP))
        {
            menu->current_entry = (menu->current_entry + menu->entry_count - 1) % menu->entry_count;
            needs_refresh = true;
        }

        else if (epro_is_key_pressed(KEY_OK))
        {
            menu->entries[menu->current_entry].callback();
            if (menu->auto_quit)
                return;

            needs_refresh = true;
        }

        else if (epro_is_key_pressed(KEY_BACK))
            return;

        if (needs_refresh)
        {
            lcd_printf(0, menu->caption);
            lcd_printf(1, "> %s", menu->entries[menu->current_entry].text);

            needs_refresh = false;
        }
    }
}


void epro_poll_keys()
{
#if EPRO_BOARD_REVISION >= 9
    volatile_key_state = PINA;
#endif

    old_key_state = current_key_state;
    current_key_state = volatile_key_state;
}


bool epro_is_key_pressed(key_t key)
{
    bool is_down = current_key_state & key;
    bool was_down = old_key_state & key;

    return (!is_down && was_down);
}


result_t epro_send_packet(const packet_t *packet)
{
    result_t result = RESULT_FAILED;

    current_interface.initialize_tx(current_bitrate_hint);
    result = _epro_send_packet(packet);
    current_interface.shutdown();

    return result;
}


result_t epro_read_packet(packet_t *packet)
{
    result_t result = RESULT_FAILED;

    current_interface.initialize_rx(current_bitrate_hint);
    result = _epro_read_packet(packet);
    current_interface.shutdown();

    return result;
}


result_t epro_send_message(const message_t *message)
{
    result_t result = RESULT_FAILED;

    packet_t *packets = 0;
    uint16_t num_packets = 0;
    message_to_packets(message, &packets, &num_packets);
    if (!packets)
        return RESULT_ERROR;

    current_interface.initialize_tx(current_bitrate_hint);
    for (uint16_t i = 0; i < num_packets; ++i)
    {
        uint8_t attempts = 0;
        result = RESULT_FAILED;
        while (result == RESULT_FAILED)
        {
            if (++attempts >= 3)
                break;

            result = _epro_send_packet(&packets[i]);
            _delay_ms(2);
        }

        if (result != RESULT_SUCCESS)
            break;
    }
    current_interface.shutdown();

    free(packets);
    return result;
}


result_t epro_read_message(message_t *message)
{
    result_t result = RESULT_FAILED;

    packet_t *packets = 0;
    uint8_t packet_count = 0;
    uint8_t packet_index = 0;

    packet_t packet;

    bool done = false;

    current_interface.initialize_rx(current_bitrate_hint);
    while (!done)
    {
        uint8_t attempts = 0;
        result = RESULT_FAILED;
        while (result == RESULT_FAILED)
        {
            if (++attempts >= 3)
                break;

            result = _epro_read_packet(&packet);
        }

        if (result != RESULT_SUCCESS)
            break;

        uint8_t index = packet_get_index(&packet);
        uint8_t total = packet_get_total(&packet);

        if (index == 1)
        {
            if (packets)
                free(packets);

            packet_index = 0;
            packet_count = total;
            packets = malloc(packet_count * sizeof(packet_t));
        }

        if (packets)
        {
            memcpy(&packets[packet_index++], &packet, sizeof(packet_t));
            if (packet_index != index)
            {
                result = RESULT_FAILED;
                done = true;
            }
            else if (packet_index >= packet_count)
            {
                result = RESULT_SUCCESS;
                done = true;
            }
        }
    }
    current_interface.shutdown();

    if (result == RESULT_SUCCESS)
        message_from_packets(message, packets, packet_count);

    if (packets)
        free(packets);

    return result;
}


void _epro_initialize_lcd()
{
    lcd_initialize();
    lcd_clear();
}


void _epro_initialize_keys()
{
    // Set key pins to input
    DDRA = ~(KEY_UP | KEY_BACK | KEY_OK| KEY_DOWN);

#if EPRO_BOARD_REVISION < 9

    // Enable internal pull-ups
    PORTA = (KEY_UP | KEY_BACK | KEY_OK | KEY_DOWN);

    // Enable CTC mode
    TCCR0 = (1<<WGM01);

    // Prescale timer clock by 1024
    TCCR0 |= (1<<CS02) | (1<<CS00);

    // Set compare value, once this value is reached, counter is reset
    // timer_cycles_per_ms = F_CPU / 1024 / 1000 = 8;
    uint8_t timer_cycles_per_ms = 8;
    OCR0 = timer_cycles_per_ms - 1;

    // Enable output compare match interrupt
    TIMSK |= (1<<OCIE0);

#else

    // Newer board revisions use external pull-downs and are debounced in hardware
    PORTA = 0x00;

#endif // EPRO_BOARD_REVISION < 9
}


result_t _epro_send_packet(const packet_t *packet)
{
    // Start timer
    timer_t timer;
    timer_start(&timer);

    result_t result = RESULT_SUCCESS;

    current_interface.send_packet(packet);
    while (!current_interface.status->done)
    {
        epro_poll_keys();
        if (epro_is_key_pressed(KEY_BACK))
            result = RESULT_ABORTED;
        else if (timer.msecs > 1000)
            result = RESULT_TIMEOUT;

        if (result != RESULT_SUCCESS)
        {
            current_interface.abort();
            goto done;
        }
    }

    result = current_interface.status->result;

done:
    timer_stop(&timer);
    return result;
}


result_t _epro_read_packet(packet_t *packet)
{
    current_interface.read_packet(packet);
    while (!current_interface.status->done)
    {
        epro_poll_keys();
        if (epro_is_key_pressed(KEY_BACK))
        {
            current_interface.abort();
            return RESULT_ABORTED;
        }
    }

    return current_interface.status->result;
}
