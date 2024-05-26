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

#include "spi.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>

#include <string.h>

typedef enum
{
    _SPI_MODE_PACKET_TX,
    _SPI_MODE_PACKET_RX,
    _SPI_MODE_ACK_TX,
    _SPI_MODE_ACK_RX,
    _SPI_MODE_IDLE

} _spi_mode_t;


static volatile _spi_mode_t mode = _SPI_MODE_IDLE;
static volatile uint8_t ack = ASCII_NACK;

static uint8_t packet_buffer[sizeof(packet_t)];
static uint8_t packet_buffer_position = 0;

static packet_t *current_packet = 0;

static interface_status_t status;


// Driver functions
static void _spi_initialize_tx(bitrate_hint_t hint);
static void _spi_initialize_rx(bitrate_hint_t hint);
static void _spi_shutdown(void);

static void _spi_send_packet(const packet_t *packet);
static void _spi_read_packet(packet_t *packet);
static void _spi_abort(void);

// Private functions
static void _spi_enable_interrupt(void);
static void _spi_disable_interrupt(void);


// Serial transfer complete interrupt
ISR(SPI_STC_vect)
{
    switch (mode)
    {
        case _SPI_MODE_PACKET_TX:
        {
            if (packet_buffer_position < sizeof(packet_t))
                SPDR = packet_buffer[packet_buffer_position++];
            else
            {
                mode = _SPI_MODE_ACK_RX;

                // Give slave some time & read confirmation
                _delay_us(200);
                SPDR = 0x00;
            }

            break;
        }

        case _SPI_MODE_PACKET_RX:
        {
            if (packet_buffer_position >= sizeof(packet_t))
                return;

            uint8_t byte = SPDR;
            if (byte == PACKET_MAGIC_NUMBER)
                packet_buffer_position = 0;

            packet_buffer[packet_buffer_position++] = byte;
            if (packet_buffer_position >= sizeof(packet_t))
            {
                // Compute & verify checksum
                packet_t *packet = (packet_t*)packet_buffer;
                uint8_t checksum = packet_compute_checksum(packet);
                ack = (checksum == packet->checksum) ? ASCII_ACK : ASCII_NACK;

                mode = _SPI_MODE_ACK_TX;

                // Wait for master to pick up confirmation
                SPDR = ack;
            }
            else
                SPDR = 0x00;

            break;
        }

        case _SPI_MODE_ACK_TX:
        {
            _spi_disable_interrupt();
            mode = _SPI_MODE_IDLE;
            
            if (ack == ASCII_ACK && current_packet)
            {
                memcpy(current_packet, packet_buffer, sizeof(packet_t));
                current_packet = 0;
            }

            status.result = (ack == ASCII_ACK) ? RESULT_SUCCESS : RESULT_FAILED;
            status.done = true;

            break;
        }

        case _SPI_MODE_ACK_RX:
        {
            ack = SPDR;

            _spi_disable_interrupt();
            mode = _SPI_MODE_IDLE;

            status.result = (ack == ASCII_ACK) ? RESULT_SUCCESS : RESULT_FAILED;
            status.done = true;

            break;
        }

        default:
            break;
    }
}


void spi_alloc_interface(interface_driver_t *interface)
{
    interface->initialize_tx = _spi_initialize_tx;
    interface->initialize_rx = _spi_initialize_rx;
    interface->shutdown      = _spi_shutdown;

    interface->send_packet   = _spi_send_packet;
    interface->read_packet   = _spi_read_packet;
    interface->abort         = _spi_abort;

    interface->status        = &status;
}


void _spi_initialize_tx(bitrate_hint_t hint)
{
    // Set ^SS, MOSI and SCK to output, MISO to input
    DDRB |= (1<<DDB4) | (1<<DDB5) | (1<<DDB7);
    DDRB &= ~(1<<DDB6);

    // Pull ^SS low to activate slave
    PORTB &= ~(1<<PB4);

    // Enable SPI & select master mode
    SPCR = (1<<SPE) | (1<<MSTR);

    // Set clock rate, see ATmega32 datasheet page 137
    switch (hint)
    {
        // F_CPU/4
        case BITRATE_HINT_FAST_REGULAR:
        case BITRATE_HINT_FAST_ABERRANT:
            SPCR &= ~((1<<SPR0) | (1<<SPR1));
            break;

        // F_CPU/128
        default:
            SPCR |= ((1<<SPR0) | (1<<SPR1));
    }

    // Clear SPI interrupt flag
    SPSR; SPDR;
}


void _spi_initialize_rx(bitrate_hint_t hint)
{
    // Set MISO to output, others to input
    DDRB |= (1<<DDB6);
    DDRB &= ~((1<<DDB4) | (1<<DDB5) | (1<<DDB7));

    // Enable SPI, slave mode is default (MSTR = 0)
    SPCR = (1<<SPE);

    // Clear SPI interrupt flag
    SPSR; SPDR;
}


void _spi_shutdown()
{
    // Disable SPI
    SPCR = 0x00;
}


void _spi_send_packet(const packet_t *packet)
{
    // Initialize packet buffer
    memcpy(packet_buffer, packet, sizeof(packet_t));
    packet_buffer_position = 0;
    mode = _SPI_MODE_PACKET_TX;

    // Start transmission
    status.result = RESULT_FAILED;
    status.done = false;

    _spi_enable_interrupt();

    // Send dummy byte to start
    SPDR = 0x00;
}


void _spi_read_packet(packet_t *packet)
{
    // Initialize packet_buffer
    memset(packet_buffer, 0, sizeof(packet_t));
    packet_buffer_position = 0;
    mode = _SPI_MODE_PACKET_RX;

    // Start transmission
    status.result = RESULT_FAILED;
    status.done = false;
    current_packet = packet;

    _spi_enable_interrupt();

    SPDR = 0x00;
}


void _spi_abort()
{
    _spi_disable_interrupt();
    mode = _SPI_MODE_IDLE;
}


void _spi_enable_interrupt()
{
    // Enable interrupt
    SPCR |= (1<<SPIE);
    
    // Clear interrupt flag
    SPSR; SPDR;
}


void _spi_disable_interrupt()
{
    SPCR &= ~(1<<SPIE);
}
