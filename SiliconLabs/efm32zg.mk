# GECKO_SDK_SUITE defined in ./silabs.mk

CMSIS_device_header = em_device.h

DEVICE = $(GECKO_SDK_SUITE)/platform/Device/SiliconLabs/EFM32ZG

CPPFLAGS += -I$(DEVICE)/Include

VPATH += $(DEVICE)/Source $(DEVICE)/Source/GCC

DEVICE_SRCS = system_efm32zg.c startup_efm32zg.c

LDSCRIPT = $(DEVICE)/Source/GCC/efm32zg.ld

CPPFLAGS += -I$(GECKO_SDK_SUITE)/platform/CMSIS/Include

# eof

