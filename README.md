# Structured Fault Handling on ARM Cortex M Processors

+ Fault Dumps

+ Fault Dump API

+ Building The Library

+ Running Test Applications

+ Quiz - Match Faulting Code Against Dump

## Fault Dumps

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

The fault dump is just a multi-line string. It is prepared at program
start, and the register value *holes* filled in at fault time to
complete the string. This minimizes string processing after fault
occurrence.

The dump above is from a faulting Cortex M3/4. On a Cortex-M0/M0+, the
hfsr through bfar registers would be missing.

The dump string can then be *exported* from the microcontroller for
fault dump analysis.  The library described here just creates the
fault dump. The export step is environment-specific, so is added by
the application developer, via a callback api.

The simplest export might be *printf* (whatever that means on a
microcontroller!):

```
void printfDumpProcessor(void) {
  printf( "%s\n", faultDump );
}
```

I work in oceanographic instrumentation, including underwater
profiling floats. If my system faults, 2000m down somewhere in the
Pacific Ocean, there ain't anyone there watching, so console printf is
of limited utility. And the JTAG cable doesn't quite reach. And the
wi-fi is spotty. And I have no hard disk.

My export strategy therefore is to declare the faultDump buffer in a
.noInit section of RAM, re-boot after the fault, wait until I am back
at the surface, then beam the fault dump home via Iridium sat comms.
Only then can the fault analysis begin, and in my (painful)
experience, each and every register in the dump can contribute to
solving the error cause.

Analysis is essentially a case of matching register values from the
fault dump against your .map and .lst files for the .bin file
installed on the faulting system. You do *have* the .map and .lst
files? If no, stop reading now, go fix your build process, then
proceed. The Makefile included here shows how I derive the .map and
.lst files for any application, adapt as necessary.

## Fault Dump API

A minimal working example of this api would be an application like:

```
#include "faultHandling.h"

static char faultDumpBuffer[FAULT_HANDLING_DUMP_SIZE];

static void noopDumpProcessor(void) {
  // Wot, no peripherals to send the faultDump to, not even a serial port!
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

/* params: text.min, text.max, msp.max, psp.max/0 */
faultHandlingSetCallStackParameters( 0, &__etext, &__StackTop, 0 );
```

You just supply lower and upper bounds for the .text section, and
upper bounds for Main Stack and Process Stack (latter optional). I get
`__etext` and `__StackTop` declared by my linker, via the .ld
file. Your build will have similar.

Exact accuracy for the values is not critical, it will just reduce false
positive identification of pushed LR values (and thus call stack
composition).  See the code for more details.

## Building The Library

### Prerequisites 

My preferred build system uses gcc tools, woven together with a
Makefile. I normally work on Linux, so my setup would be

```
$ sudo apt install gcc-arm-none-eabi make
```

The code here is being built by make. Adapt to your build
system as needed.

### For Generic ARM Processors

We can build this library against a generic ARM Cortex M processor,
i.e. one with no peripherals defined. To do so, we need some ARM
header files relating to a generic ARM device, plus the CMSIS Core
headers, i.e:

```
CMSIS_5/Device/ARM/ARMCM0/Include/*.h
CMSIS_5/Device/ARM/ARMCM3/Include/*.h
...
CMSIS_5/CMSIS/Core/Include/*.h
```

To build any test cases that use the library, we can also make use of ARM
system and startup files, e.g.

```
CMSIS_5/Device/ARM/ARMCM3/Source/*.c
CMSIS_5/Device/ARM/ARMCM3/Source/GCC/*.S, *.ld
```

Assuming that the source tree above is not already on your development
system, grab it from GitHub:

```
$ cd someDir
$ git clone https://github.com/ARM-software/CMSIS_5.git

```

We have verified that tagged releases 5.8.0 and 5.9.0 are good to go
for our purposes. Other releases are untested. So, you could checkout
a particular commit, via e.g.

```
$ git checkout 5.8.0
$ git checkout 5.9.0
```

or just skip this step and go with the HEAD of master/main.

Next, clone the fault-handler repo (the one whose README you are now
reading), if not done so already:

```
$ cd someOtherDir
$ git clone https://github.com/tobermory/faultHandling-cortex-m.git
```

Next, edit the Makefile, setting the CMSIS_HOME variable to point to
your ARM CMSIS_5 repo clone, e.g:

```
$ ed Makefile

CMSIS_HOME = /path/to/my/someDir/CMSIS_5
```

With all the prep work done, it should now be a case of just:

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
libfaultHandling_CM3.a
```


By default, the build output is terse (uncluttered!).  To see a bit
more:

```
$ make clean
$ make V=1
```

We include a make target that just prints important variables
(VPATH, CPPFLAGS, etc):

```
$ make flags
```

There are a few simple demo applications:

```
$ make tests
```

Each demo/test produces four files: .axf, .bin, .map, .lst.

### Other Cortex M Variants

Default build is for Cortex M3, my usual target. To build for other CPUs:

```
$ make clean
$ make CM4=1
$ make CM4=1 tests

$ make clean
$ make CM0=1
$ make CM0=1 tests
```

### For Vendor-Specific Micro-controllers

I work with Cortex M micro-controllers from Silicon Labs. I have two
of their starter kits upon which I can test this fault handling
library:

+ The [EFM32GG-STK3700](https://www.silabs.com/development-tools/mcu/32-bit/efm32gg-starter-kit), which has a Cortex M3 Giant Gecko, with 1MB flash and 128kb RAM.

+ The [EFM32ZG-STK3200](https://www.silabs.com/development-tools/mcu/32-bit/efm32zg-starter-kit), which has a Cortex M0+ Zero Gecko, 32kb flash and just 4kb RAM.

These boards have real peripherals, so we can truly export our fault
dumps to e.g. a serial console. To make use of such peripherals, we
just need SiliconLabs's HAL layer, which they call *emlib*.  We also
want/need their Device files, which replace the ARM ones we built
against in the previous section.

SiliconLabs's preferred code distribution appears not to be via
GitHub, but via their IDE called SimplicityStudio.  Once that is
installed, you should find a file system layout akin to:

```
/path/to/mySimplicityStudio
```

under which you find this directory

```
developer/sdks/gecko_sdk_suite/v2.X/platform
```

and under there you find

```
CMSIS/Include/*.h

// for the stk3700
Device/SiliconLabs/EFM32GG/Include/*.h
Device/SiliconLabs/EFM32GG/Source/*.c
Device/SiliconLabs/EFM32GG/Source/GCC/*.c, *.ld

// for the stk3200
Device/SiliconLabs/EFM32ZG/Include/*.h
Device/SiliconLabs/EFM32ZG/Source/*.c
Device/SiliconLabs/EFM32ZG/Source/GCC/*.c, *.ld
```

With such a file layout, we can build our fault handling library AND
some test cases that would run on those boards and produce real fault
dumps exported via a serial console.


Proceed thus:

```
$ cd SiliconLabs

$ ed silabs.mk
GECKO_SDK_SUITE=/path/to/my/SimplicityStudio/developer/sdks/gecko_sdk_suite/v2.X
```

i.e. the `GECKO_SDK_SUITE` variable matches your SimplicityStudio
install path.  Then, to build the library and some test applications
for the STK3700 starter board, so we can turn this:

```
src/test/c/*.c
```

to this:

```
SiliconLabs/stk3700/*.bin
```

we do this:

```
$ cd stk3700
$ make lib tests
$ ls *.bin
busFault.bin  iaccviol.bin  invstate.bin  mpuFault.bin  stackSmashing.bin
```

As well as the .bin file, we of course want the corresponding listing
and map file, for these are what we cross-reference when analyzing a
fault dump.  These are built automatically:

```
$ ls busFault.*
busFault.axf busFault.bin busFault.lst busFault.map
```

Similarly, we can build some test applications for the STK3200:

```
$ cd ../stk3200
$ make lib tests
$ ls *.bin
iaccviol.bin  invstate.bin  stackSmashing.bin
```

See [stk3700.c](src/test/c/stk3700.c) and
[stk3200.c](src/test/c/stk3200.c) for instructions on making use of a
uart peripheral on each board (via the Expansion Header) to export our
fault dumps to a host machine via serial/usb.

The STK3700 and STK3200 boards have a Segger JLink debugger interface
built in. So, you just need Segger's JLinkExe (Linux) program to flash
these binaries to the boards and run them. Our Makefiles provide
targets to do this, e.g:

```
$ cd SiliconLabs/stk3700/

$ make busFault.run
```

will invoke JLinkExe to flash busFault.bin to the stk3700 and run
it. If your serial uart on the STK3700 is hooked up to a host machine,
you can see the dump via something as simple as `cat`:

$ stty -F /dev/ttyUSB0 115200 -echo
$ cat /dev/ttyUSB0
```

Should you ever need to actually debug these test applications, and
you have Segger's Ozone debugger installed, that too can be invoked
via make, e.g.

```
$ make busFault.ozone
```

## Quiz - Match Faulting Code Against Dump

Can you identify five faulting programs from their fault dumps?

When built via the Makefiles included here and run on my SiliconLabs
STK3700 starter board, the test applications included here make these
mistakes (i.e. fault):

+ Access Violation

+ Invalid State

+ MPU Violation

+ Bus Fault

+ Stack Corruption (Smashing)

Here are relevant code snippets, i.e. the source of the faults:

```
1

void (*p)(void) = (void(*)(void))((1LL << 32) - 1);
p();

2

void (*p)(void) = (void(*)(void))0;
p();

3

static void bar(void) {
}

static void foo(void) {
  uint32_t a[1];
  bar();
  a[0] = 0xCAFEBABE;
  a[1] = 0xDEADBEEF;
  a[2] = 0xCAFEBABE;
  a[3] = 0xDEADBEEF;
}

4

void (*p)(void) = (void(*)(void)) 0x20202020;
p();

5

MPU_RegionInit_TypeDef mpuInit = MPU_INIT_FLASH_DEFAULT;
mpuInit.size = mpuRegionSize32b;
mpuInit.accessPermission = mpuRegionNoAccess;
MPU_ConfigureRegion(&mpuInit);
MPU_Enable(MPU_CTRL_PRIVDEFENA);

int* p = (int*)NULL;
int i = *p;
```

which produce, in no particular order, these fault dump strings:

```
A

r7    DEADBEEF
sp    2001FFD8
excrt FFFFFFF9
psr   20000003
hfsr  40000000
cfsr  00000001
mmfar E000ED34
bfar  E000ED38
shcsr 00000000
s.r0  4000C400
s.r1  0000000A
s.r2  0000000A
s.r3  DEADBEEF
s.r12 2000056A
s.lr  00000267
s.pc  CAFEBABE
s.psr 00000000
2001FFFC 0000016B
00000000 00000000
00000000 00000000
00000000 00000000

B

r7    2001FFF0
sp    2001FFD0
excrt FFFFFFF9
psr   20000003
hfsr  40000000
cfsr  00000100
mmfar E000ED34
bfar  E000ED38
shcsr 00000000
s.r0  00000000
s.r1  00003588
s.r2  200005D4
s.r3  20202020
s.r12 2000056A
s.lr  0000022F
s.pc  20202020
s.psr 00000000
2001FFFC 0000016B
00000000 00000000
00000000 00000000
00000000 00000000

C

r7    2001FFD8
sp    2001FFB8
excrt FFFFFFF9
psr   20000004
hfsr  00000000
cfsr  00000082
mmfar 00000000
bfar  00000000
shcsr 00010001
s.r0  00000004
s.r1  00000000
s.r2  E000ED00
s.r3  00000000
s.r12 2000056A
s.lr  00000275
s.pc  0000027A
s.psr 41000000
2001FFD8 00000001
2001FFE4 00000001
2001FFE8 00000101
2001FFFC 0000016B

D

r7	  2001FFF0
sp    2001FFD0
excrt FFFFFFF9
psr   20000003
hfsr  40000000
cfsr  00000001
mmfar E000ED34
bfar  E000ED38
shcsr 00000000
s.r0  00000000
s.r1  00003698
s.r2  200005DC
s.r3  FFFFFFFF
s.r12 20000572
s.lr  00000303
s.pc  FFFFFFFE
s.psr 01000000
2001FFFC 0000016B
00000000 00000000
00000000 00000000
00000000 00000000

E


r7    2001FFF0
sp    2001FFD0
excrt FFFFFFF9
psr   20000003
hfsr  40000000
cfsr  00020000
mmfar E000ED34
bfar  E000ED38
shcsr 00000000
s.r0  00000000
s.r1  00003588
s.r2  200005D4
s.r3  00000000
s.r12 2000056A
s.lr  0000022D
s.pc  00000000
s.psr 40000000
2001FFFC 0000016B
00000000 00000000
00000000 00000000
00000000 00000000
```

Which is which ?  Answers on a postcard...

## Fault Guru

Joking aside, we could collect all the heuristics we use when
identifying faulting source code from its register dump and create
some kind of 'fault guru' program.  You feed the guru a fault dump and
it tells you what happened, and perhaps even suggests a code fix.
Something along the lines of

```
$ faultGuru myFaultDumpString.txt

Your program crashed!
```

but hopefully a bit more specific than that!

