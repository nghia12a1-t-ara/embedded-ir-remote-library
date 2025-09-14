# Makefile for ATTiny13 IR Decoder Project
# 
# Usage:
#   make          - Build the project
#   make clean    - Clean build files
#   make program  - Program the ATTiny13 (requires avrdude setup)

# Project Configuration
PROJECT = attiny13_ir_decoder
MCU = attiny13
F_CPU = 9600000UL
PROGRAMMER = usbasp

# Compiler Configuration
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AVRDUDE = avrdude

# Compiler Flags
CFLAGS = -Wall -Wextra -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections

SOURCES = main.c ir_decoder.c ir_common.c attiny13_hal.c
OBJECTS = $(SOURCES:.c=.o)

# Fuse Settings
FUSE_L = 0x7A
FUSE_H = 0xFF

# Default Target
all: $(PROJECT).hex size

# Compile C files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files to create ELF file
$(PROJECT).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

# Create HEX file from ELF file
$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Display size information
size: $(PROJECT).elf
	$(SIZE) --format=avr --mcu=$(MCU) $<

# Program the microcontroller
program: $(PROJECT).hex
	$(AVRDUDE) -p $(MCU) -c $(PROGRAMMER) -U flash:w:$<:i

# Set fuses
fuses:
	$(AVRDUDE) -p $(MCU) -c $(PROGRAMMER) -U lfuse:w:$(FUSE_L):m -U hfuse:w:$(FUSE_H):m

transmitter: SOURCES += ir_transmitter.c
transmitter: $(PROJECT).hex

# Clean build files
clean:
	rm -f $(OBJECTS) $(PROJECT).elf $(PROJECT).hex

# Phony targets
.PHONY: all clean program fuses size transmitter
