PROJECT = irtimer
SRC_DIR = ./src
INC_DIR = ./include
BUILD_DIR = ./bin


#SHARED_DIR += src
#SHARED_DIR += include

CFILES = $(shell find $(SRC_DIR)/ -type f -name *.c)
HFILES = $(shell find $(INC_DIR)/ -type f -name *.h)
INCLUDES += $(patsubst %,-I%, $(INC_DIR))
#CFILES += %.c
#CFILES += api.c
#AFILES += api-asm.S

# TODO - you will need to edit these two lines!
#DEVICE=stm32f030f4p6
DEVICE_TARGET=stm32
DEVICE_FAMILY=f1
DEVICE_VARIANT=03cbt6
DEVICE=$(DEVICE_TARGET)$(DEVICE_FAMILY)$(DEVICE_VARIANT)
#DEVICE=stm32f103c6t6
#DEVICE=stm32f051c8t6
#OOCD_FILE = board/stm32f0discovery.cfg
	
PROJECT_CLEAN = $(RM) -r ./*.elf ./*.bin $(BUILD_DIR)/*

RTOS_NAME = freertos
RTOS_DIR = $(RTOS_NAME)
RTOS_SRCDIR = $(RTOS_DIR)/src
RTOS_INCDIR = $(RTOS_DIR)/include
RTOS_BINDIR = $(RTOS_DIR)/bin
RTOS_LIBDIR = $(RTOS_DIR)/lib
RTOS_LIB = $(RTOS_LIBDIR)/lib$(RTOS_NAME).a
RTOS_SRCS = $(wildcard $(RTOS_SRCDIR)/*.c)
RTOS_OBJS = $(patsubst $(RTOS_SRCDIR)/%.c, $(RTOS_BINDIR)/%.o, $(wildcard $(RTOS_SRCDIR)/*.c))
RTOS_CLEAN = $(RM) $(RTOS_LIBDIR)/* $(RTOS_BINDIR)/* 

OOCD_FILE = board/stm32f103c8_blue_pill.cfg
OPENCM3_DIR = ./libopencm3
OPENCM3_LIB = $(OPENCM3_DIR)/lib/libopencm3_$(DEVICE_TARGET)$(DEVICE_FAMILY).a
OPENCM3_MAKE = $(MAKE) -C $(OPENCM3_DIR) TARGETS=$(DEVICE_TARGET)/$(DEVICE_FAMILY)
OPENCM3_CLEAN = $(OPENCM3_MAKE) clean

all:
ifeq ($(wildcard $(OPENCM3_LIB)),)
	$(OPENCM3_MAKE)
	$(MAKE) all
else
# You shouldn't have to edit anything below here.
#VPATH += $(SHARED_DIR)
#INCLUDES += $(patsubst %,-I%, $(SHARED_DIR))

include $(OPENCM3_DIR)/mk/genlink-config.mk
include mk/rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

endif

.PHONY clean:
	$(OPENCM3_CLEAN)
	$(RTOS_CLEAN)
	$(PROJECT_CLEAN)	