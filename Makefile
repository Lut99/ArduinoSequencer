# MAKEFILE
#   by Lut99
#
# Created:
#   13/08/2021, 11:40:41
# Last edited:
#   13/08/2021, 16:24:10
# Auto updated?
#   Yes
#
# Description:
#   Makefile for the squencer project.
#

### DEFINES
# The arduino directory
ARDUINO_DIR := ${HOME}/Arduino/arduino-1.8.15
# Path of the arduino core library
LIBRARY_DIR = $(ARDUINO_DIR)/hardware/arduino/avr/cores/arduino

# The bin directory
BIN_DIR = bin
# The obj firectory
OBJ_DIR = $(BIN_DIR)/obj
# The elf firectory
ELF_DIR = $(BIN_DIR)/elf

# The C compiler
CC = "$(ARDUINO_DIR)/hardware/tools/avr/bin/avr-gcc"
# The C++ compiler
CPP = "$(ARDUINO_DIR)/hardware/tools/avr/bin/avr-g++"
# Static library archiver
AR = "$(ARDUINO_DIR)/hardware/tools/avr/bin/avr-ar"
# Linker?
OBJ_COPY = "$(ARDUINO_DIR)/hardware/tools/avr/bin/avr-objcopy"

# CPU ID?
F_CPU = 16000000L
# Processor type, I think
MCU = atmega328p
# Arduino ID?
ARDUINO = 10813

# Flags needed for both (C & C++) compilers
GENERAL_FLAGS = -c -g -Os -Wall -Wextra -ffunction-sections -fdata-sections -mmcu=$(MCU) -DF_CPU=$(F_CPU) -MMD -DUSB_VID=null -DUSB_PID=null -DARDUINO=$(ARDUINO) -DARDUINO_AVR_UNO -DARDUINO_ARCH_AVR
CC_FLAGS = $(GENERAL_FLAGS)
CPP_FLAGS = $(GENERAL_FLAGS) -fno-exceptions

# Include paths
INCLUDES = "-I$(ARDUINO_DIR)/hardware/arduino/avr/cores/arduino" "-I$(ARDUINO_DIR)/hardware/arduino/avr/variants/standard"



### COMPILE LISTS
# Core library files to compile
CORE_FILES := $(shell find $(LIBRARY_DIR) -name '*.c') $(shell find $(LIBRARY_DIR) -name '*.cpp')
CORE_FILES := $(CORE_FILES:$(LIBRARY_DIR)/%.c=$(OBJ_DIR)/core/%.o)
CORE_FILES := $(CORE_FILES:$(LIBRARY_DIR)/%.cpp=$(OBJ_DIR)/core/%.o)



### PHONY RULES
# Declare which rules are phony
.PHONY: default all clean core sequencer upload
default: all

all: core sequencer
clean:
	rm -rf $(BIN_DIR)/*

dirs: $(BIN_DIR) $(OBJ_DIR) $(OBJ_DIR)/core $(ELF_DIR)

core: $(OBJ_DIR)/core/libcore.a

sequencer_4: $(BIN_DIR)/sequencer_4.eep $(BIN_DIR)/sequencer_4.hex

upload: $(BIN_DIR)/sequencer_4.hex
	avrdude "-C$(ARDUINO_DIR)/hardware/tools/avr/etc/avrdude.conf" -v -v -v -v -patmega328p -carduino -P/dev/ttyATM0 -b57600 -D -Uflash:w:$<:i



### DIRECTORY RULES
$(BIN_DIR):
	mkdir -p $@
$(OBJ_DIR):
	mkdir -p $@
$(OBJ_DIR)/core:
	mkdir -p $@
$(ELF_DIR):
	mkdir -p $@



### BUILD RULES
# Build a .c file in the arduino core
$(OBJ_DIR)/core/%.o: $(LIBRARY_DIR)/%.c | dirs
	$(CC) $(CC_FLAGS) $(INCLUDES) -o $@ $<

# Build a .cpp file in the arduino core
$(OBJ_DIR)/core/%.o: $(LIBRARY_DIR)/%.cpp | dirs
	$(CPP) $(CPP_FLAGS) $(INCLUDES) -o $@ $<

$(OBJ_DIR)/core/libcore.a: $(CORE_FILES) | dirs
	$(AR) rcs $@ $^



# Build local .ino files
$(OBJ_DIR)/%.o: %.ino | dirs
	$(CPP) $(CPP_FLAGS) -include Arduino.h $(INCLUDES) -o $@ -x c++ $<

# Create the .elf with all code, essentially linking it
$(ELF_DIR)/sequencer_4.elf: $(OBJ_DIR)/sequencer_4.o $(OBJ_DIR)/core/libcore.a | dirs
	$(CC) -Os -Wl,--gc-sections -mmcu=$(MCU) -o $@ $^ -lm

# Generate an eeprom from the .elf
$(BIN_DIR)/sequencer_4.eep: $(ELF_DIR)/sequencer_4.elf | dirs
	$(OBJ_COPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $@

# Generate a binary file from the .elf
$(BIN_DIR)/sequencer_4.hex: $(ELF_DIR)/sequencer_4.elf | dirs
	$(OBJ_COPY) -O ihex -R .eeprom $< $@
