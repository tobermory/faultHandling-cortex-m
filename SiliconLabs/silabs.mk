GECKO_SDK_SUITE ?= $(HOME)/Downloads/SimplicityStudio_v4/developer/sdks/gecko_sdk_suite/v2.6

VENDOR = SiliconLabs

EMLIB = $(GECKO_SDK_SUITE)/platform/emlib

CPPFLAGS += -I$(EMLIB)/inc

VPATH += $(EMLIB)/src

# eof
