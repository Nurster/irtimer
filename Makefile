PROJECT = irtimer
BUILD_DIR = ./bin


SHARED_DIR += src
SHARED_DIR += include
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

