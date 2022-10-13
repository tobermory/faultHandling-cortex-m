CMSIS_device_header = ARMCM0plus.h

DEVICE = $(CMSIS_HOME)/Device/ARM/ARMCM0plus

CPPFLAGS += -I$(DEVICE)/Include -DARMCM0P

VPATH += $(DEVICE)/Source $(DEVICE)/Source/GCC

DEVICE_SRCS = system_ARMCM0plus.c startup_ARMCM0plus.c

LDSCRIPT = $(DEVICE)/Source/GCC/gcc_arm.ld

CPPFLAGS += -I$(CMSIS_HOME)/CMSIS/Core/Include

# eof
