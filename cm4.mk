LIB = faultHandling_CM4.a

# For purpose of CPU fault handling, CM4 equivalent to CM3 ??
LIB_ASM_SRCS = faultHandling_cm3.S

# Set this mandatory CC setting here, NOT in CFLAGS, which the user
# likes to control (warnings,debug,etc).
CPU_OPTIONS += -mcpu=cortex-m4

# Our local CPPFLAGS, used by faultHandling.c
CPPFLAGS += -D__CORTEX_M4

# eof
