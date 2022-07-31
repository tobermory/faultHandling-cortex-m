CMSIS_DEVICE = $(CMSIS_HOME)/Device/ARM/ARMCM3

CPPFLAGS += -I$(CMSIS_DEVICE)/Include -DARMCM3

ifndef VENDOR

VPATH += $(CMSIS_DEVICE)/Source $(CMSIS_DEVICE)/Source/GCC

DEVICE_SRCS = system_ARMCM3.c startup_ARMCM3.c

LDSCRIPT = $(CMSIS_DEVICE)/Source/GCC/gcc_arm.ld

endif

# eof
