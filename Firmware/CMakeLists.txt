####################################################################################################
#                                                                                                  #
#   This file is part of the ePro firmware.                                                        #
#                                                                                                  #
#   Author:                                                                                        #
#   Marcel Hasler <mahasler@gmail.com>                                                             #
#                                                                                                  #
#   Copyright (c) 2010 - 2014                                                                      #
#   Bonn-Rhein-Sieg University of Applied Sciences                                                 #
#                                                                                                  #
#   This program is free software: you can redistribute it and/or modify it under the terms        #
#   of the GNU General Public License as published by the Free Software Foundation, either         #
#   version 3 of the License, or (at your option) any later version.                               #
#                                                                                                  #
#   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      #
#   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      #
#   See the GNU General Public License for more details.                                           #
#                                                                                                  #
#   You should have received a copy of the GNU General Public License along with this program.     #
#   If not, see <https://www.gnu.org/licenses/>.                                                   #
#                                                                                                  #
####################################################################################################

#  Generate Makefile by running
#
#  mkdir build && cd build
#  cmake -DCMAKE_TOOLCHAIN_FILE=../AVRToolchain.cmake ..
#
#  Supported make commands:
#
#  make         = Build the firmware
#  make clean   = Clean out built project files
#  make fuses   = Set correct AVR fuse bits, using avrdude
#  make program = Download the firmware to the device, using avrdude

cmake_minimum_required(VERSION 3.5)
project(ePro)

set(MCU "atmega32")
set(F_CPU "8000000")

set(ePro_SRC
    src/epro.c
    src/i2c.c
    src/irda.c
    src/lcd.c
    src/main.c
    src/message.c
    src/messagetable.c
    src/packet.c
    src/rs232.c
    src/scrolltext.c
    src/settings.c
    src/spi.c
    src/timer.c
    src/uart.c
    src/util.c

    src/support/lcd.c
)

include_directories(/usr/avr/include ${CMAKE_BINARY_DIR})

# Set compiler and linker flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmcu=${MCU}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DF_CPU=${F_CPU}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map,--cref")

# Build message table
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/_message_table.inc
                   COMMAND python ${CMAKE_SOURCE_DIR}/src/scripts/mkmsg.py
                                  ${CMAKE_SOURCE_DIR}/src/resource/messages.txt
                                  ${CMAKE_BINARY_DIR}/_message_table.inc)

add_custom_target(messages DEPENDS ${CMAKE_BINARY_DIR}/_message_table.inc)

# Create ELF
add_executable(${CMAKE_PROJECT_NAME}.elf ${ePro_SRC})
add_dependencies(${CMAKE_PROJECT_NAME}.elf messages)

# Create HEX
add_custom_command(TARGET  ${CMAKE_PROJECT_NAME}.elf POST_BUILD
                   COMMAND ${OBJCOPY_COMMAND} -O ihex -R .eeprom
                           ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
                   COMMENT "Creating ${CMAKE_PROJECT_NAME}.hex")

# Create EEP
add_custom_command(TARGET  ${CMAKE_PROJECT_NAME}.elf POST_BUILD
                   COMMAND ${OBJCOPY_COMMAND} -j .eeprom --set-section-flags=.eeprom=alloc,load
                                                         --change-section-lma .eeprom=0 -O ihex
                           ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.eep
                   COMMENT "Creating ${CMAKE_PROJECT_NAME}.eep")

# Create LSS
add_custom_command(TARGET  ${CMAKE_PROJECT_NAME}.elf POST_BUILD
                   COMMAND ${OBJDUMP_COMMAND} -h -S ${CMAKE_PROJECT_NAME}.elf > ${CMAKE_PROJECT_NAME}.lss
                   COMMENT "Creating ${CMAKE_PROJECT_NAME}.lss")

# Create SYM
add_custom_command(TARGET  ${CMAKE_PROJECT_NAME}.elf POST_BUILD
                   COMMAND ${NM_COMMAND} -n ${CMAKE_PROJECT_NAME}.elf > ${CMAKE_PROJECT_NAME}.sym
                   COMMENT "Creating ${CMAKE_PROJECT_NAME}.sym")

# Print size
add_custom_command(TARGET  ${CMAKE_PROJECT_NAME}.elf POST_BUILD
                   COMMAND ${SIZE_COMMAND} ${CMAKE_PROJECT_NAME}.elf
                   COMMENT "Size:")

# Fuse bits as given by Engbedded Fuse Calculator
set(AVRDUDE_FUSE -U lfuse:w:0xcc:m -U hfuse:w:0xd9:m)

# Write command
set(AVRDUDE_WRITE -U flash:w:${CMAKE_PROJECT_NAME}.hex -U eeprom:w:${CMAKE_PROJECT_NAME}.eep)

# Lock memory, 0x3c = 00111100
# LB1 & LB2 (bits 0 & 1) are programmed, bits 6 & 7 aren't used
set(AVRDUDE_LOCK -U lock:w:0x3c:m)

# General flags
set(AVRDUDE_FLAGS -p ${MCU} -P usb -c avrisp2 -v)

# Program and lock memory
add_custom_target(program COMMAND avrdude ${AVRDUDE_FLAGS} ${AVRDUDE_WRITE} ${AVRDUDE_LOCK}
                          DEPENDS ${CMAKE_PROJECT_NAME}.elf)

# Set fuse bits
add_custom_target(fuses COMMAND avrdude ${AVRDUDE_FLAGS} ${AVRDUDE_FUSE})
