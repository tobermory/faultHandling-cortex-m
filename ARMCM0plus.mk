CMSIS_DEVICE = $(CMSIS_HOME)/Device/ARM/ARMCM0plus

VPATH += $(CMSIS_DEVICE)/Source $(CMSIS_DEVICE)/Source/GCC

DEVICE_SRCS = system_ARMCM0plus.c startup_ARMCM0plus.c

CPPFLAGS += -I$(CMSIS_DEVICE)/Include -DARMCM0P

LDSCRIPT = $(CMSIS_DEVICE)/Source/GCC/gcc_arm.ld

# eof
