CMSIS_device_header = ARMCM3.h

DEVICE = $(CMSIS_HOME)/Device/ARM/ARMCM3

CPPFLAGS += -I$(DEVICE)/Include -DARMCM3

VPATH += $(DEVICE)/Source $(DEVICE)/Source/GCC

DEVICE_SRCS = system_ARMCM3.c startup_ARMCM3.c

LDSCRIPT = $(DEVICE)/Source/GCC/gcc_arm.ld

CPPFLAGS += -I$(CMSIS_HOME)/CMSIS/Core/Include

# eof
