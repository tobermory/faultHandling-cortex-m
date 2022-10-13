LIB = libfaultHandling_CM3.a

LIB_ASM_SRCS = faultHandling_cm3.S

# Set this mandatory CC setting here, NOT in CFLAGS, which the user
# likes to control (warnings,debug,etc).
CPU_OPTIONS += -mcpu=cortex-m3

# eof
