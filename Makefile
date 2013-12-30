# Convect v1.0
# (c) 2013 D.M.

MCU=atmega32u4
F_CPU=16000000L
VID=0x2341
PID=0x8041
USB_PRODUCT='"Arduino Yun"'
USB_MANUFACTURER=
BOARD=AVR_YUN
ARCH=ARCH_AVR
IDE_VERSION=154

PROJECT_PATH ?= $(HOME)/Arduino
ARDUINO_PATH ?= $(PROJECT_PATH)
BUILD_PATH  := build

APP = calib335z

CROSS   = $(ARDUINO_PATH)/hardware/tools/avr/bin/avr-
CXX     = g++
CC      = gcc
LD      = gcc
AR      = ar
OBJCOPY = objcopy

OPTIM   = -Os

CFLAGS  = $(OPTIM) -w -fno-exceptions -MMD
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -DARDUINO=$(IDE_VERSION)
CFLAGS += -DARDUINO_$(BOARD) -DARDUINO_$(ARCH_AVR)
CFLAGS += -DUSB_VID=$(VID) -DUSB_PID=$(PID) 
CFLAGS += -DUSB_MANUFACTURER=$(USB_MANUFACTURER) 
CFLAGS += -DUSB_PRODUCT=$(USB_PRODUCT)

LDFLAGS = $(OPTIM) -Wl,--gc-sections -mmcu=$(MCU)

EEPFLAGS  = -O ihex -j .eeprom 
EEPFLAGS += --set-section-flags=.eeprom=alloc,load 
EEPFLAGS += --no-change-warnings --change-section-lma .eeprom=0

HEXFLAGS = -O ihex -R .eeprom

ARFLAGS = rcs

INCLUDE  = -I$(ARDUINO_PATH)/hardware/arduino/avr/cores/arduino 
INCLUDE += -I$(ARDUINO_PATH)/hardware/arduino/avr/variants/yun 

LIB_CXXSRC  = src/Mailbox.cpp
LIB_CXXSRC += src/YunClient.cpp
LIB_CXXSRC += src/YunServer.cpp
LIB_CXXSRC += src/Process.cpp
LIB_CXXSRC += src/FileIO.cpp
LIB_CXXSRC += src/Bridge.cpp
LIB_CXXSRC += src/Console.cpp
LIB_CXXSRC += src/HttpClient.cpp

CORE_PREFIX = $(ARDUINO_PATH)/hardware/arduino/avr/cores/arduino/
LIBRARIESW = $(wildcard $(ARDUINO_PATH)/libraries/*)
LIBRARIES  = $(foreach dir, $(LIBRARIESW), $(shell test -d $(dir) && echo $(dir)))
colon := :
empty :=
space := $(empty) $(empty)
LIB_PATH = $(subst $(space),$(colon),$(LIBRARIES))
LIB_INC = $(addprefix -I, $(addsuffix /src, $(LIBRARIES)))
INCLUDE += $(LIB_INC)

VPATH = $(CORE_PREFIX):$(CORE_PREFIX)/avr-libc:$(LIB_PATH)

CORE_CSRC  = wiring.c
CORE_CSRC += wiring_shift.c
CORE_CSRC += WInterrupts.c
CORE_CSRC += realloc.c
CORE_CSRC += malloc.c
CORE_CSRC += wiring_pulse.c
CORE_CSRC += hooks.c
CORE_CSRC += wiring_analog.c
CORE_CSRC += wiring_digital.c

CORE_CXXSRC  = WString.cpp
CORE_CXXSRC += Stream.cpp
CORE_CXXSRC += IPAddress.cpp
CORE_CXXSRC += CDC.cpp
CORE_CXXSRC += HID.cpp
CORE_CXXSRC += WMath.cpp
CORE_CXXSRC += new.cpp
CORE_CXXSRC += Print.cpp
CORE_CXXSRC += HardwareSerial.cpp
CORE_CXXSRC += Tone.cpp
CORE_CXXSRC += main.cpp
CORE_CXXSRC += USBCore.cpp

CORE_OBJS  = $(CORE_CSRC:.c=.o)
CORE_OBJS += $(CORE_CXXSRC:.cpp=.o)
LIB_OBJS   = $(LIB_CXXSRC:.cpp=.o)

LIBSt = $(LIB_OBJS) cores.a
LIBS  = $(addprefix $(BUILD_PATH)/, $(LIBSt))

DEPENDSt  = $(CORE_OBJS:.o=.d)
DEPENDSt += $(LIB_OBJS:.o=.d)
DEPENDS   = $(addprefix $(BUILD_PATH)/, $(DEPENDSt))

LIB_O=$(patsubst %.o,$(BUILD_PATH)/%.o,$(LIB_OBJS)) 
CORE_O=$(patsubst %.o,$(BUILD_PATH)/%.o,$(CORE_OBJS)) 

all: $(BUILD_PATH)/$(APP).hex

-include $(DEPENDS)

$(BUILD_PATH)/src:
	mkdir -p $(BUILD_PATH)/src

$(BUILD_PATH)/$(APP).hex: $(BUILD_PATH)/$(APP).elf 

$(BUILD_PATH)/$(APP).elf: $(BUILD_PATH)/src $(BUILD_PATH)/cores.a $(LIB_O) $(BUILD_PATH)/$(APP).o

$(BUILD_PATH)/cores.a: $(CORE_O)
	$(CROSS)$(AR) $(ARFLAGS) $@ ${CORE_O}

$(BUILD_PATH)/%.o: %.cpp
	$(CROSS)$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@
    
$(BUILD_PATH)/%.o: %.c
	$(CROSS)$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
    
$(BUILD_PATH)/%.elf: $(BUILD_PATH)/%.o
	$(CROSS)$(LD) $(LDFLAGS) -o $@ $< $(LIBS) -L$(BUILD_PATH)/ -lm

$(BUILD_PATH)/%.hex: $(BUILD_PATH)/%.elf
	$(CROSS)$(OBJCOPY) $(HEXFLAGS) $< $@

$(BUILD_PATH)/%.eep: $(BUILD_PATH)/%.elf
	$(CROSS)$(OBJCOPY) $(EEPFLAGS) $< $@

.PHONY: clean

clean:
	@rm ${BUILD_PATH}/*.o ${BUILD_PATH}/*.elf ${BUILD_PATH}/*.d ${BUILD_PATH}/*.hex ${BUILD_PATH}/src/*.o
