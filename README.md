# CPU Fault Handling on ARM Cortex M Processors

For programs running on ARM Cortex-M series microcontrollers, given a
CPU fault resulting from code such as this

```
void (*p)(void) = (void(*)(void))0;
...
p();
```
this 'fault handling' library produces a 'fault dump' like this:

```
r7    2001BEC8
sp    2001BEA0
excrt FFFFFFFD
psr   20000004
hfsr  00000000
cfsr  00000082
mmfar 00000000
bfar  00000000
shcsr 00010001
s.r0  00000000
s.r1  0004BCF8
s.r2  00000000
s.r3  0004BCF8
s.r12 01010101
s.lr  0001D053
s.pc  0003B9CC
s.psr 21000000
```

The dump can then 'exported' from the microcontroller for fault dump
analysis.  Fault dump creation is done here. The export step is
environment-specific, so is added by the application developer.

The simplest 'export' might be printf (whatever that means on a
microcontroller!):

```
void printfDumpProcessor(void) {
  printf( "%s\n", faultDump );
}
```

See the [sources](src/main/) for more details.  It's just one .h and one .c.

## Prerequisites 

My preferred build system uses gcc tools, woven together with a
Makefile. I normally work on Linux, so my setup would be

```
$ sudo apt install gcc-arm-none-eabi
$ sudo apt install make
```
Adapt to your build system as needed. 

## Building the Library

First, and only if needed, clone the ARM CMSIS_5 repository.
It includes core Cortex-M header files we need:

```
$ cd someDir
$ git clone https://github.com/ARM-software/CMSIS_5.git

```

Next, clone this repo (the one whose README you are now reading), if
not done so already:

```
$ cd someOtherDir
$ git clone https://github.com/tobermory/faultHandling-cortex-m
```

Next, edit the Makefile, setting the CMSIS_HOME variable to point to
your ARM CMSIS repo clone, e.g:

```
CMSIS_HOME = /path/to/my/someDir/CMSIS_5
```

With all the prep work, it should now be a case of just:

```
$ make
```

to go from this:

```
src/main
├── asm
│   └── faultHandling_cm3.S
├── c
│   ├── faultHandling.c
└── include
    ├── faultHandling.h
```

to this:

```
faultHandling_CM3.a
```


By default, the build output is terse (uncluttered!).  To see a bit
more:

```
$ make clean
$ make V=1
```

There are a few simple demo applications:

```
$ make tests
```

Each demo/test produces four files: .axf, .bin, .map, .lst.

## Other CPUs

Default build is for Cortex M3, my usual target. To build for other CPUs:

```
$ make clean
$ make CM4=1
$ make CM4=1 tests

$ make clean
$ make CM0=1
$ make CM0=1 tests
```

## The API

A minimal working example of this api would be an application like:

```
#include "faultHandling.h"

static char faultDumpBuffer[FAULT_HANDLING_DUMP_SIZE];

static void noopDumpProcessor(void) {
  // Wot, no periperhals to send the faultDump to, not even a serial port!
}

int main(void) {
  faultHandlingSetDumpProcessor( faultDumpBuffer, noopDumpProcessor );
  faultHandlingSetPostFaultAction( POSTHANDLER_LOOP );

  rest of application, will fault somewhere...
}

// Override the (weak) HFH to vector to our library
__attribute__((naked))
void HardFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

```

In additional to the core CPU register values, the fault dump can
include an inferred function call stack, i.e. which function call
sequence led to the fault.  To do this, the library needs some help on
memory layout from the user:

```
extern uint32_t __etext;
extern uint32_t __StackTop;
   
faultHandlingSetCallStackParameters( (uint32_t*)4,
									   &__etext,
									   &__StackTop,
									   0 );
```

You just supply lower and upper bounds for the .text section, and
upper bounds for Main Stack and Process Stack (latter optional).

## For Real Micro-controllers

WIP

### The SiliconLabs STK3700, a starter kit for EFM32 Giant Geckos

$ cd SiliconLabs/stk3700

$ make
$ make tests

produces stk3700.axf, stk3700.bin, stk3700.map, stk3700.lst. Flash the
.bin file to an available STK3700, connect up the serial console pins
to your host (likely USB port), fire up minicom/teraterm and you
should harvest the fault dump.

## The SiliconLabs STK3200, a starter kit for EFM32 Zero Geckos

$ cd SiliconLabs/stk3200

$ make
$ make tests

produces stk3200.axf, stk3200.bin, stk3200.map, stk3200.lst. Flash the
.bin file to an available STK3200, connect up the serial console pins
to your host (likely USB port), fire up minicom/teraterm and you
should harvest the fault dump.


## Fault Guru

OK, given a dump:

```
r7    2001BEC8
sp    2001BEA0
excrt FFFFFFFD
psr   20000004
hfsr  00000000
cfsr  00000082
mmfar 00000000
bfar  00000000
shcsr 00010001
s.r0  00000000
s.r1  0004BCF8
s.r2  00000000
s.r3  0004BCF8
s.r12 01010101
s.lr  0001D053
s.pc  0003B9CC
s.psr 21000000
```

how do we infer what actually went wrong? What clues does the dump reveal?

'faultGuru' collects some heuristics together in the form of a C
program that runs on a non-embedded host, e.g. Linux, Windows, etc.
It reads in the dump produced by our faultHandling api, and prints
facts and suggestions gleaned from the dump.

Of course, you'll want the .lst file that accompanied the .bin file
build.  You DID create the .lst file, didn't you ?!