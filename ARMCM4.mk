CMSIS_DEVICE = $(CMSIS_HOME)/Device/ARM/ARMCM4

CPPFLAGS += -I$(CMSIS_DEVICE)/Include -DARMCM4

ifndef VENDOR

VPATH += $(CMSIS_DEVICE)/Source $(CMSIS_DEVICE)/Source/GCC

DEVICE_SRCS = system_ARMCM4.c startup_ARMCM4.c

LDSCRIPT = $(CMSIS_DEVICE)/Source/GCC/gcc_arm.ld

endif

# eof
