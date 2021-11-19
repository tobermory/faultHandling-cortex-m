#

For programs running on ARM Cortex-M series microcontrollers, given a
fault like this

```
void (*p)(void) = (void(*)(void))0;
p();
```
this library produces a 'fault dump' like this:

DUMP

which is then 'exported' from the microcontroller for fault dump
analysis.  Fault dump creation is done here, the export is added by
the application developer. The simplest 'export' might be printf
(whatever that means on a microcontroller):

```
void printfDumpProcessor(void) {
  printf( "%s\n", faultDump );
}
```

# Building the Library

```
$ make
```

produces faultHandling_CM3.a

```
$ make tests
```
produces noopProcessor.axf, .bin, .map, .lst

Default build is for Cortex M3, my usual target. To build for other CPUs:

```
$ make clean
$ make CM4=1
$ make CM4=1 tests

$ make clean
$ make CM0=1
$ make CM0=1 tests
```

## For real micro=controllers

## The SiliconLabs STK3700, a starter kit for EFM32 Giant Geckos

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

OK, given a dump

DUMP

how do we infer what actually went wrong? What clues does the dump reveal?

'faultGuru' collects some heuristics together in the form of a C
program that runs on a non-embedded host, e.g. Linux, Windows, etc.
It reads in the dump produced by our faultHandling api, and prints
facts and suggestions gleaned from the dump.

Of course, you'll want the .lst file that accompanied the .bin file
build.  You DID create the .lst file, didn't you ?!