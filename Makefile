.DELETE_ON_ERROR:
.SUFFIXES:

# Makefile                                                                      
# Written by Leon Krieg <info@madcow.dev>

ARCH      := m32
FREQ      := 1600000UL
MCU       := atmega32a
ASP       := usbasp
GCC       := avr-gcc
AVD       := avrdude
RMF       := rm -f
BINDIR    := bin
ELFILE    := $(BINDIR)/out.elf
TARGET    := $(BINDIR)/core.hex
CFLAGS    := -Os -std=c99 -Wall -Wextra -Werror
CPPFLAGS  := -DF_CPU=$(FREQ)

.PHONY: all
all: flash

.PHONY: flash
flash: $(TARGET)
	$(AVD) -c $(ASP) -p $(ARCH) -U flash:w:$(TARGET)

$(TARGET): src/main.c
	$(GCC) -o $(ELFILE) $(CFLAGS) $(CPPFLAGS) $^ -mmcu=$(MCU)
	avr-objcopy -j .text -j .data -O ihex $(ELFILE) $@
	avr-size --format=avr --mcu=$(MCU) $@

clean:
	$(RMF) $(TARGET)
	$(RMF) $(ELFILE)
