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
#include "uart.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>

#include <string.h>

// 500000 baud is the highest rate possible, equals ubrr = 0
#define MAX_BAUDRATE 500000L

#define _UART_PIN_RXD 0
#define _UART_PIN_TXD 1

#if EPRO_BOARD_REVISION < 9
#define _UART_PIN_IR_ENABLE 2
#else
#define _UART_PIN_IR_ENABLE 5
#endif

interface_status_t _uart_status;

typedef enum
{
    UART_MODE_PACKET_TX,
    UART_MODE_PACKET_RX,
    UART_MODE_ACK_TX,
    UART_MODE_ACK_RX,
    UART_MODE_IDLE

} _uart_mode_t;


static volatile _uart_mode_t mode = UART_MODE_IDLE;
static volatile uint8_t ack = ASCII_NACK;
static volatile bool ack_sent = false;

static uint8_t packet_buffer[sizeof(packet_t)];
static uint8_t packet_buffer_position = 0;

static packet_t *current_packet = 0;


// Private functions
static void _uart_enable_tx(void);
static void _uart_disable_tx(void);
static void _uart_enable_rx(void);
static void _uart_disable_rx(void);


// Tx register empty interrupt
ISR(USART_UDRE_vect)
{
    if (mode == UART_MODE_PACKET_TX)
    {
        if (packet_buffer_position < sizeof(packet_t))
            UDR = packet_buffer[packet_buffer_position++];
        else
        {
            _uart_disable_tx();
            mode = UART_MODE_ACK_RX;
            _uart_enable_rx();
        }
    }

    else if (mode == UART_MODE_ACK_TX)
    {
        if (!ack_sent)
        {
            UDR = ack;
            ack_sent = true;
        }
        else
        {
            _uart_disable_tx();
            mode = UART_MODE_IDLE;

            if (ack == ASCII_ACK && current_packet)
            {
                memcpy(current_packet, packet_buffer, sizeof(packet_t));
                current_packet = 0;
            }

            _uart_status.result = (ack == ASCII_ACK) ? RESULT_SUCCESS : RESULT_FAILED;
            _uart_status.done = true;
        }
    }
}


// Rx complete interrupt
ISR(USART_RXC_vect)
{
    if (mode == UART_MODE_PACKET_RX)
    {
        if (packet_buffer_position >= sizeof(packet_t))
            return;

        uint8_t byte = UDR;
        if (byte == PACKET_MAGIC_NUMBER)
            packet_buffer_position = 0;

        packet_buffer[packet_buffer_position++] = byte;
        if (packet_buffer_position >= sizeof(packet_t))
        {
            _uart_disable_rx();

            // Compute checksum
            packet_t *packet = (packet_t*)packet_buffer;
            uint8_t checksum = packet_compute_checksum(packet);
            ack = (checksum == packet->checksum) ? ASCII_ACK : ASCII_NACK;
            ack_sent = false;

            mode = UART_MODE_ACK_TX;
            _uart_enable_tx();
        }
    }

    else if (mode == UART_MODE_ACK_RX)
    {
        ack = UDR;

        _uart_disable_rx();
        mode = UART_MODE_IDLE;

        _uart_status.result = (ack == ASCII_ACK) ? RESULT_SUCCESS : RESULT_FAILED;
        _uart_status.done = true;
    }
}


void _uart_initialize()
{
    // Initialize port D as output
    DDRD |= (1<<_UART_PIN_TXD) | (1<<_UART_PIN_RXD) | (1<<_UART_PIN_IR_ENABLE);
    PORTD &= ~((1<<_UART_PIN_TXD) | (1<<_UART_PIN_RXD) | (1<<_UART_PIN_IR_ENABLE));

    // Enable transmitter & receiver
    UCSRB = (1<<TXEN) | (1<<RXEN);

    // Set frame format to 8N1
    UCSRC = (1<<URSEL) | (1<<UCSZ0) | (1<<UCSZ1);

    // Clear rx buffer
    do UDR; while (UCSRA & (1<<RXC));
}


void _uart_set_baudrate(uint32_t baudrate)
{
    // See ATmega32 datasheet pages 141-143
    uint16_t ubrr = ((float)F_CPU / (16*baudrate)) - 1;
    UBRRH = (uint8_t)(ubrr>>8);
    UBRRL = (uint8_t)ubrr;
}


void _uart_set_ir_enabled(bool enable)
{
    if (enable)
    {
        PORTD |= (1<<_UART_PIN_IR_ENABLE);

        // Wait at least 1000 * T_OSC
        _delay_ms(1);
    }
    else
        PORTD &= ~(1<<_UART_PIN_IR_ENABLE);
}


void _uart_shutdown(void)
{
    // Disable receiver and transmitter
    UCSRB = 0x00;
}


void _uart_send_byte(uint8_t byte)
{
    while (!(UCSRA & (1<<UDRE)))
        ;

    UDR = byte;

    while (!(UCSRA & (1<<UDRE)))
        ;
}


void _uart_send_packet(const packet_t *packet)
{
    // Initialize packet buffer
    memcpy(packet_buffer, packet, sizeof(packet_t));
    packet_buffer_position = 0;
    mode = UART_MODE_PACKET_TX;

    // Start transmission
    _uart_status.result = RESULT_FAILED;
    _uart_status.done = false;

    _uart_enable_tx();
}


void _uart_read_packet(packet_t *packet)
{
    // Initialize packet_buffer
    memset(packet_buffer, 0, sizeof(packet_t));
    packet_buffer_position = 0;
    mode = UART_MODE_PACKET_RX;

    // Clear receive buffer
    do UDR; while (UCSRA & (1<<RXC));

    // Wait for incoming transmission
    _uart_status.result = RESULT_FAILED;
    _uart_status.done = false;
    current_packet = packet;

    _uart_enable_rx();
}


void _uart_abort()
{
    _uart_disable_tx();
    _uart_disable_rx();

    mode = UART_MODE_IDLE;
}


void _uart_enable_tx()
{
    UCSRB |= (1<<UDRIE);
}


void _uart_disable_tx()
{
    UCSRB &= ~(1<<UDRIE);
}


void _uart_enable_rx()
{
    UCSRB |= (1<<RXCIE);
}


void _uart_disable_rx()
{
    UCSRB &= ~(1<<RXCIE);
}
