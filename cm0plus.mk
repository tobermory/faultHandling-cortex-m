LIB = faultHandling_CM0.a

LIB_ASM_SRCS = faultHandling_cm0.S

# Set this mandatory CC setting here, NOT in CFLAGS, which the user
# likes to control (warnings,debug,etc)
CPU_OPTIONS += -mcpu=cortex-m0plus

# Our local CPPFLAGS, used by faultHandling.c
CPPFLAGS += -D__CORTEX_M0PLUS

# eof
