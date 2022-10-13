#
# Copyright © 2022 Stuart Maclean
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER NOR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#

# Stuart Maclean, 2021.  See ./LICENSE.txt and ./README.md

BASEDIR ?= $(abspath .)

## Infrastructure, just to ensure this Makefile is leaner

include $(BASEDIR)/toolchain.mk

########################### CMSIS BUNDLE FROM ARM ############################

# ARM Software's CMSIS_5 repo at github cloned to some local directory.
# Replace the CMSIS_HOME path with your local equivalent.

CMSIS_HOME = $(BASEDIR)/../CMSIS_5

################################### CPU #################################

# We're building here for M3 by default, you could switch in e.g. M0, M4
# (make CM4=1 or edit below).

ifdef CM0
include $(BASEDIR)/cm0plus.mk
else ifdef CM4
include $(BASEDIR)/cm4.mk
else
include $(BASEDIR)/cm3.mk
endif

# A VENDOR-specific build (see e.g. ./SiliconLabs/*) will define its
# own DEVICE files. If no VENDOR, use ARM defaults, which describe a
# generic CPU only (no peripherals).

ifndef CMSIS_device_header

ifdef CM0
include $(BASEDIR)/ARMCM0plus.mk
else ifdef CM4
include $(BASEDIR)/ARMCM4.mk
else
include $(BASEDIR)/ARMCM3.mk
endif

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
TESTS += noopProcessor
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

# Needed by faultHandling.h
CPPFLAGS += -DCMSIS_device_header=\"$(CMSIS_device_header)\"

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

# Build ALL configurations
sweep:
	$(MAKE) clean lib tests CM3=1
	$(MAKE) clean lib tests CM4=1
	$(MAKE) clean lib tests CM0=1
	$(MAKE) -C SiliconLabs/stk3700 clean lib tests
	$(MAKE) -C SiliconLabs/stk3200 clean lib tests


.PHONY: default lib clean distclean flags tests sweep

# eof
