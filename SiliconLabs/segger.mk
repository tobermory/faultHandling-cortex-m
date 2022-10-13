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
