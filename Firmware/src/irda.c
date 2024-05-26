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
#include "irda.h"
#include "uart.h"

#include <avr/io.h>

#include <util/delay.h>

// See MCP2120 datasheet page 7
#define _IRDA_CMD_CHANGE_RATE 0x11

// Pins
#if EPRO_BOARD_REVISION < 9
#define _IRDA_PIN_IR_MODE  3
#define _IRDA_PIN_IR_RESET 4
#else
#define _IRDA_PIN_IR_MODE  6
#define _IRDA_PIN_IR_RESET 7
#endif

static const uint8_t commands[BITRATE_HINT_COUNT] =
{
    0x87, //   9600 / BITRATE_HINT_SLOW_REGULAR
    0x87, //   9600 / BITRATE_HINT_SLOW_ABERRANT
    0x81, // 115200 / BITRATE_HINT_FAST_REGULAR
    0x81  // 115200 / BITRATE_HINT_FAST_ABERRANT
};

static const uint32_t bitrates[BITRATE_HINT_COUNT] =
{
    9600,   // BITRATE_HINT_SLOW_REGULAR
    9600,   // BITRATE_HINT_SLOW_ABERRANT
    115200, // BITRATE_HINT_FAST_REGULAR
    115200, // BITRATE_HINT_FAST_ABERRANT
};

// Driver function
static void _irda_initialize(bitrate_hint_t hint);
static void _irda_shutdown(void);

// Private functions
static void _irda_set_bitrate(bitrate_hint_t hint);
static void _irda_reset(void);


void irda_alloc_interface(interface_driver_t *interface)
{
    interface->initialize_tx = _irda_initialize;
    interface->initialize_rx = _irda_initialize;
    interface->shutdown      = _irda_shutdown;

    interface->send_packet   = _uart_send_packet;
    interface->read_packet   = _uart_read_packet;
    interface->abort         = _uart_abort;

    interface->status        = &_uart_status;
}


void _irda_initialize(bitrate_hint_t hint)
{
    // Initialize port D pins as output
    DDRD |= (1<<_IRDA_PIN_IR_MODE) | (1<<_IRDA_PIN_IR_RESET);
    PORTD &= ~((1<<_IRDA_PIN_IR_MODE) | (1<<_IRDA_PIN_IR_RESET));

    // Initialize UART & enable IR endec
    _uart_initialize();
    _uart_set_ir_enabled(true);

    // Reset the endec
    _irda_reset();

    // Set bitrate
    _irda_set_bitrate(hint);
}


void _irda_shutdown()
{
    // Wait a little in case the transmission isn't yet finished
    _delay_ms(1);
    _uart_set_ir_enabled(false);
    _uart_shutdown();
}


void _irda_set_bitrate(bitrate_hint_t hint)
{
    // Set default baudrate
    _uart_set_baudrate(9600);

    // Pull MODE pin low to bring the chip into
    // command mode & allow for propagation delay.
    PORTD &= ~(1<<_IRDA_PIN_IR_MODE);
    _delay_us(100);

    // Send the appropriate commands
    _uart_send_byte(commands[hint]);
    _uart_send_byte(_IRDA_CMD_CHANGE_RATE);

    // Pull MODE pin high to bring the chip into data mode
    PORTD |= (1<<_IRDA_PIN_IR_MODE);
    _delay_us(100);

    // Set requested baudrate
    _uart_set_baudrate(bitrates[hint]);
}


void _irda_reset(void)
{
    // See MCP2120 datasheet page 19 for timing details

    // Pull reset pin low for at least 2 us
    PORTD |= (1<<_IRDA_PIN_IR_RESET);
    _delay_us(10);

    // Pull reset pin high and wait 18 ms
    PORTD &= ~(1<<_IRDA_PIN_IR_RESET);
    _delay_ms(18);
}
