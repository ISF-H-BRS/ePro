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

#include "i2c.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include <util/delay.h>

#include <string.h>

#define SLAVE_ADDRESS 0x01

#define SLAVE_W  (SLAVE_ADDRESS << 1)
#define SLAVE_R  (SLAVE_W | 0x01)

// Master Transmitter
#define MT_BUS_ERROR   0x00
#define MT_START       0x08
#define MT_R_START     0x10
#define MT_SLA_W_ACK   0x18
#define MT_SLA_W_NACK  0x20
#define MT_DATA_ACK    0x28
#define MT_DATA_NACK   0x30
#define MT_ARB_LOST    0x38

// Master Receiver
#define MR_BUS_ERROR   0x00
#define MR_START       0x08
#define MR_R_START     0x10
#define MR_ARB_LOST    0x38
#define MR_SLA_R_ACK   0x40
#define MR_SLA_R_NACK  0x48
#define MR_DATA_ACK    0x50
#define MR_DATA_NACK   0x58

// Slave Transmitter
#define ST_BUS_ERROR   0x00
#define ST_SLA_R_ACK   0xa8
#define ST_DATA_ACK    0xb8
#define ST_DATA_NACK   0xc0
#define ST_LDATA_ACK   0xc8

// Slave Receiver
#define SR_BUS_ERROR   0x00
#define SR_SLA_W_ACK   0x60
#define SR_DATA_ACK    0x80
#define SR_DATA_NACK   0x88
#define SR_R_START     0xa0
#define SR_STOP        0xa0

typedef enum
{
    _I2C_MODE_PACKET_TX,
    _I2C_MODE_PACKET_RX,
    _I2C_MODE_ACK_TX,
    _I2C_MODE_ACK_RX,
    _I2C_MODE_IDLE

} _i2c_mode_t;


static const uint32_t bitrates[BITRATE_HINT_COUNT] =
{
    10000,  // BITRATE_HINT_SLOW_REGULAR
    12345,  // BITRATE_HINT_SLOW_ABERRANT
    100000, // BITRATE_HINT_FAST_REGULAR
    123456, // BITRATE_HINT_FAST_ABERRANT
};


static volatile _i2c_mode_t mode = _I2C_MODE_IDLE;
static volatile uint8_t ack = ASCII_NACK;

static uint8_t packet_buffer[sizeof(packet_t)];
static uint8_t packet_buffer_position = 0;

static packet_t *current_packet = 0;

static interface_status_t status;


// Driver functions
static void _i2c_initialize_tx(bitrate_hint_t hint);
static void _i2c_initialize_rx(bitrate_hint_t hint);
static void _i2c_shutdown(void);

static void _i2c_send_packet(const packet_t *packet);
static void _i2c_read_packet(packet_t *packet);
static void _i2c_abort(void);

// Private helper, masks out prescaler bits
static uint8_t _i2c_read_status(void);

// Interrupt handlers
static void _i2c_packet_tx_isr(void);
static void _i2c_packet_rx_isr(void);
static void _i2c_ack_tx_isr(void);
static void _i2c_ack_rx_isr(void);


// TWI interrupt
ISR(TWI_vect)
{
    switch (mode)
    {
        case _I2C_MODE_PACKET_TX:
            _i2c_packet_tx_isr();
            break;

        case _I2C_MODE_PACKET_RX:
            _i2c_packet_rx_isr();
            break;

        case _I2C_MODE_ACK_TX:
            _i2c_ack_tx_isr();
            break;

        case _I2C_MODE_ACK_RX:
            _i2c_ack_rx_isr();
            break;

        default:
            break;
    }
}


void i2c_alloc_interface(interface_driver_t *interface)
{
    interface->initialize_tx = _i2c_initialize_tx;
    interface->initialize_rx = _i2c_initialize_rx;
    interface->shutdown      = _i2c_shutdown;

    interface->send_packet   = _i2c_send_packet;
    interface->read_packet   = _i2c_read_packet;
    interface->abort         = _i2c_abort;

    interface->status        = &status;
}


void _i2c_initialize_tx(bitrate_hint_t hint)
{
    // Disable internal pull-ups
    PORTC &= ~((1<<PC0) | (1<<PC1));

    // Set bitrate using 1/4 prescaler (see ATmega32 datasheet page 175)
    TWSR |= (1<<TWPS0);
    TWBR = ((float)F_CPU / (8*bitrates[hint])) - 2;
}


void _i2c_initialize_rx(bitrate_hint_t hint)
{
    // Disable internal pull-ups
    PORTC &= ~((1<<PC0) | (1<<PC1));

    // Place slave address to react on into slave address register
    TWAR = SLAVE_ADDRESS << 1; // Bits 7 to 1
}


void _i2c_shutdown()
{
    // Disable TWI
    TWCR = 0x00;
}


void _i2c_send_packet(const packet_t *packet)
{
    // Initialize packet buffer
    memcpy(packet_buffer, packet, sizeof(packet_t));
    packet_buffer_position = 0;
    mode = _I2C_MODE_PACKET_TX;

    status.result = RESULT_FAILED;
    status.done = false;

    // Enable interrupt & send START
    TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWSTA);
}


void _i2c_read_packet(packet_t *packet)
{
    // Initialize packet_buffer
    memset(packet_buffer, 0, sizeof(packet_t));
    packet_buffer_position = 0;
    mode = _I2C_MODE_PACKET_RX;

    status.result = RESULT_FAILED;
    status.done = false;
    current_packet = packet;

    // Enable interrupt and prepare to send ACK
    TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
}


void _i2c_abort()
{
    // Release TWI pins
    TWCR = (1<<TWEN);
    mode = _I2C_MODE_IDLE;
}


uint8_t _i2c_read_status()
{
    // Mask out prescaler bits
    return (TWSR & 0xf8);
}


void _i2c_packet_tx_isr()
{
    uint8_t i2c_status = _i2c_read_status();
    switch (i2c_status)
    {
        // START has been sent, send slave address
        case MT_START:
        case MT_R_START:
            TWDR = SLAVE_W;
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
            packet_buffer_position = 0;
            break;

        // Slave address or data sent, ACK received, send next data byte
        case MT_SLA_W_ACK:
        case MT_DATA_ACK:
        {
            if (packet_buffer_position < sizeof(packet_t))
            {
                TWDR = packet_buffer[packet_buffer_position++];
                TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
            }
            else
            {
                // Disable interrupt and send STOP
                TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO);

                mode = _I2C_MODE_ACK_RX;

                // Give slave some time and send START
                _delay_us(200);
                TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWSTA);
            }

            break;
        }

        // Bus arbitration lost, initiate RESTART
        case MT_ARB_LOST:
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWSTA);
            break;

        // Slave address or data sent, NACK received or bus error
        case MT_SLA_W_NACK:
        case MT_DATA_NACK:
        case MT_BUS_ERROR:
        default:
            TWCR = (1<<TWEN);
            break;
    }
}


void _i2c_packet_rx_isr()
{
    uint8_t i2c_status = _i2c_read_status();
    switch (i2c_status)
    {
        // Own address received, ACK returned
        case SR_SLA_W_ACK:
            packet_buffer_position = 0;
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
            break;

        // Data received, ACK returned
        case SR_DATA_ACK:
        {
            if (packet_buffer_position < sizeof(packet_t))
            {
                packet_buffer[packet_buffer_position++] = TWDR;
                TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);

                if (packet_buffer_position >= sizeof(packet_t))
                {
                    // Compute & verify checksum
                    packet_t *packet = (packet_t*)packet_buffer;
                    uint8_t checksum = packet_compute_checksum(packet);
                    ack = (checksum == packet->checksum) ? ASCII_ACK : ASCII_NACK;

                    mode = _I2C_MODE_ACK_TX;

                    // Wait for master to pick up confirmation
                    TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
                }
            }

            break;
        }

        // Data received, NACK returned or bus error, 
        case SR_DATA_NACK:
        case SR_BUS_ERROR:
            TWCR = (1<<TWSTO) | (1<<TWINT);
            break;

        // Unknown status, acknowledge new requests
        default:
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
            break;
    }
}


void _i2c_ack_tx_isr()
{
    uint8_t i2c_status = _i2c_read_status();
    switch (i2c_status)
    {
        // Own address received, ACK returned
        case ST_SLA_R_ACK:
            TWDR = ack;
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
            break;

        // Data sent, NACK received, disable interrupt & release pins
        case ST_DATA_NACK:
            TWCR = (1<<TWEN) | (1<<TWINT);

            mode = _I2C_MODE_IDLE;
            
            if (ack == ASCII_ACK && current_packet)
            {
                memcpy(current_packet, packet_buffer, sizeof(packet_t));
                current_packet = 0;
            }

            status.result = (ack == ASCII_ACK) ? RESULT_SUCCESS : RESULT_FAILED;
            status.done = true;

            break;

        // Bus error, release TWI pins
        case ST_BUS_ERROR:
            TWCR = (1<<TWSTO) | (1<<TWINT);
            break;

        // Unknown status, acknowledge new requests
        default:
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA);
            break;
    }
}


void _i2c_ack_rx_isr()
{
    uint8_t i2c_status = _i2c_read_status();
    switch (i2c_status)
    {
        // START has been sent, send slave address
        case MR_START:
        case MR_R_START:
            TWDR = SLAVE_R;
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
            break;

        // Slave address sent, ACK received
        case MR_SLA_R_ACK:
            TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT);
            break;

        // Data received, NACK returned, disable interrupt & send STOP
        case MR_DATA_NACK:
            ack = TWDR;

            TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWSTO);            
            mode = _I2C_MODE_IDLE;

            status.result = (ack == ASCII_ACK) ? RESULT_SUCCESS : RESULT_FAILED;
            status.done = true;

            break;

        // Bus arbitration lost, initiate RESTART
        case MR_ARB_LOST:
        case MR_SLA_R_NACK:
        case MR_BUS_ERROR:
        default:
            TWCR = (1<<TWEN);
            break;
    }
}
