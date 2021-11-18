# Stuart Maclean, 2021.  See ./LICENSE.txt and ./README.md

BASEDIR ?= $(abspath .)

## Infrastructure, just to ensure this Makefile is leaner

include $(BASEDIR)/toolchain.mk

########################### CMSIS BUNDLE FROM ARM ############################

# ARM Software's CMSIS_5 repo at github cloned to some local directory.
# Replace with YOURS!
CMSIS_HOME = $(BASEDIR)/../CMSIS_5

# CMSIS_DEVICE delegated to cm3.mk, cm4.mk, see below

################################### CPU #################################

# We're building here for M3 by default, you could switch in e.g. M0, M4
# (make CM4=1 or edit below)

ifdef CM0

include $(BASEDIR)/cm0plus.mk
include $(BASEDIR)/ARMCM0plus.mk

else ifdef CM4

include $(BASEDIR)/cm4.mk
include $(BASEDIR)/ARMCM4.mk

else

include $(BASEDIR)/cm3.mk
include $(BASEDIR)/ARMCM3.mk

endif

########################## FAULT HANDLING LIB, TESTS ######################

LIB_C_SRCS = faultHandling.c

LIB_OBJS = $(LIB_ASM_SRCS:.S=.o) $(LIB_C_SRCS:.c=.o)

DEVICE_OBJS = $(DEVICE_SRCS:.c=.o)

# LIB itself was defined in cmX.mk

# Some tests that use the faultHandling api, currently just one.
# Vendor-specific test cases will define others, see
# e.g. SiliconLabs/stk3700/Makefile.

ifndef VENDOR
TESTS = noopProcessor
endif

############################ Derived File Names #############################

# Buildable artifacts derivable from our TESTS names

AXFS = $(addsuffix .axf, $(TESTS))

BINS = $(addsuffix .bin, $(TESTS))

#################### Build Settings: VPATH, CPPFLAGS ##############

# run 'make flags' to inspect these

# Locates our lib sources
VPATH += $(BASEDIR)/src/main/c $(BASEDIR)/src/main/asm

# Locates our test sources
VPATH += $(BASEDIR)/src/test/c

# Locates our lib headers
CPPFLAGS += -I$(BASEDIR)/src/main/include

# Locates CMSIS headers, used by our lib
CPPFLAGS += -I$(CMSIS_HOME)/CMSIS/Core/Include

LDLIBS += -lc -lnosys

############################### Build Targets ################################

# Print out recipes only if V set (make V=1), else quiet to avoid clutter
ifndef V
ECHO=@
endif

default: lib

lib: $(LIB)

$(LIB): $(LIB_OBJS)
	@echo AR $(@F)
	$(ECHO)$(AR) cr $@ $^

tests: $(BINS)

clean:
	$(RM) *.bin *.axf *.map *.lst *.a *.o *.i


############################## Pattern Rules ################################

# Overriding default patterns for inclusion of CPU_OPTIONS, which are
# a must. The arm-none-eabi toolchain can compile for a wide variety of
# cpus, and we have to tell it what we have.

%.o : %.c
	@echo CC $(<F)
	$(ECHO)$(CC) -c $(CPPFLAGS) $(CPU_OPTIONS) $(CFLAGS) $< $(OUTPUT_OPTION)

%.o : %.S
	@echo AS $(<F)
	$(ECHO)$(AS) $(CPU_OPTIONS) $(ASFLAGS) $< $(OUTPUT_OPTION)

%.bin: %.axf 
	@echo OBJCOPY $(<F) = $(@F)
	$(ECHO)$(OBJCOPY) -O binary $< $@

# The .map file is a product of the .axf build. In addition, build
# the .lst file too, it is vital in fault dump analysis.
$(AXFS) : %.axf : %.o $(LIB) $(DEVICE_OBJS)
	@echo LD $(@F) = $(^F)
	$(ECHO)$(CC) $(LDFLAGS) $(CPU_OPTIONS) -T $(LDSCRIPT) \
	-Xlinker -Map=$*.map $^ $(LDLIBS) $(OUTPUT_OPTION)
	@echo OBJDUMP $(@F) = $*.lst
	$(ECHO)$(OBJDUMP) -dl $@ > $*.lst

################################### MISC ##############################

# Inspect VPATH, CPPFLAGS, useful when things won't build
flags:
	@echo
	@echo VPATH    $(VPATH)
	@echo
	@echo CPPFLAGS $(CPPFLAGS)
	@echo
	@echo LIB $(LIB)
	@echo
	@echo LIB_OBJS $(LIB_OBJS)

.PHONY: default lib clean distclean flags tests

# eof
