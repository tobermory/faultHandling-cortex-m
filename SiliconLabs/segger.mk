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

# make targets to invoke SEGGER tools JLinkExe (flashing)
# and ozone (debugging).

# To flash and run a program called foo (i.e. its binary is foo.bin):

# make foo.run

# To debug a program called foo (i.e. its binary is foo.bin):

# make foo.ozone

JLINK = JLinkExe

RUNS = $(addsuffix .run, $(TESTS))

$(RUNS) : %.run : %.jlink %.bin
	$(JLINK) $<

%.jlink:
	-$(RM) $@
	@echo if SWD >> $@
	@echo device $(PART_NUMBER) >> $@
	@echo speed 1000 >> $@
	@echo loadfile $*.bin >> $@
	@echo rnh >> $@
	@echo q >> $@

.PHONY: $(RUNS)

OZONE = ozone

OZONES = $(addsuffix .ozone, $(TESTS))

%.jdebug : ozone.jdebug.in
	@echo SED $(@F)
	@cp $< $@
	$(ECHO)sed -i 's/DEVICE/$(PART_NUMBER)/g' $@
	$(ECHO)sed -i 's/PROGRAM/$*.axf/g' $@

$(OZONES) : %.ozone: %.axf %.jdebug
	$(OZONE) $*.jdebug

%.ozone: CFLAGS += -g -gdwarf-2 -O0

.PHONY: $(OZONES)
