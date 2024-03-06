PROJECT = ili9163
BUILD_DIR = ./bin

RTOS_NAME = freertos
RTOS_DIR = ./$(RTOS_NAME)
RTOS_SRCDIR = $(RTOS_DIR)/src
RTOS_INCDIR = $(RTOS_DIR)/include
RTOS_BINDIR = $(RTOS_DIR)/bin
RTOS_LIBDIR = $(RTOS_DIR)/lib
RTOS_LIB = $(RTOS_LIBDIR)/lib$(RTOS_NAME).a
#RTOS_SRCS = $(patsubst $(RTOS_SRCDIR)/%.c, %.c, $(wildcard $(RTOS_SRCDIR)/*.c))
#RTOS_OBJS = $(RTOS_SRCS:%.c=%.o)
RTOS_SRCS = $(wildcard $(RTOS_SRCDIR)/*.c)
RTOS_OBJS = $(patsubst $(RTOS_SRCDIR)/%.c, $(RTOS_BINDIR)/%.o, $(wildcard $(RTOS_SRCDIR)/*.c))

#RTOS_DIRS = $(shell find $(RTOS_DIR) -type d -print)
#RTOS_OBJS = $(patsubst $(RTOS_DIR)/%.c, $(RTOS_BUILD_DIR)/%.o, $(RTOS_CFILES))
#RTOS_OBJS = $(patsubst %.c, %.o, $(RTOS_CFILES))

SHARED_DIR += ./src
SHARED_DIR += ./include
CFILES = main.c
#CFILES += %.c
#CFILES += api.c
#AFILES += api-asm.S

# TODO - you will need to edit these two lines!
#DEVICE=stm32f030f4p6
DEVICE=stm32f103cbt6
#DEVICE=stm32f103c6t6
#DEVICE=stm32f051c8t6
#OOCD_FILE = board/stm32f0discovery.cfg
OOCD_FILE = board/stm32f103c8_blue_pill.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, $(SHARED_DIR))
OPENCM3_DIR=./libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include mk/rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

