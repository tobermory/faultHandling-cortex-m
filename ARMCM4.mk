CMSIS_device_header = ARMCM4.h

DEVICE = $(CMSIS_HOME)/Device/ARM/ARMCM4

CPPFLAGS += -I$(DEVICE)/Include -DARMCM4

VPATH += $(DEVICE)/Source $(DEVICE)/Source/GCC

DEVICE_SRCS = system_ARMCM4.c startup_ARMCM4.c

LDSCRIPT = $(DEVICE)/Source/GCC/gcc_arm.ld

CPPFLAGS += -I$(CMSIS_HOME)/CMSIS/Core/Include

# eof
