
# $ sudo apt install gcc-arm-none-eabi

# $ sudo apt install binutils-arm-none-eabi  (??)

# $ sudo apt install libnewlib-arm-none-eabi (??)

PREFIX  = arm-none-eabi

AR	= $(PREFIX)-ar
AS	= $(PREFIX)-as
CC	= $(PREFIX)-gcc
LD	= $(PREFIX)-ld
NM	= $(PREFIX)-nm
OBJCOPY	= $(PREFIX)-objcopy
OBJDUMP	= $(PREFIX)-objdump
RANLIB	= $(PREFIX)-ranlib
READELF	= $(PREFIX)-readelf
SIZE	= $(PREFIX)-size
STRIP	= $(PREFIX)-strip

