/**
 * Copyright © 2022 Stuart Maclean
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER NOR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */
#include <stdio.h>

#include "faultHandling.h"

#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

/**
 * @author Stuart Maclean
 *
 * As per ./stk3700.c in terms of console printf setup.

 * The fault generation here is via stack smashing. A called function
 * declares local variables, but then writes past them, into slots in
 * the stack from which r7 and lr will be popped by function epilog.
 */

void initConsole(void);
void consoleWrite( char* s );

// A place to hold the formatted fault dump, of correct size.
static char faultDumpBuffer[FAULT_HANDLING_DUMP_SIZE];

/**
 * Dump the fault to the serial console, so a user can 'see' what went wrong.
 */
void consoleDumpProcessor(void) {
  //  consoleWrite( "foo\r\n" );
  consoleWrite( faultDumpBuffer );
}

static void foo(void);

int main(void) {

  CHIP_Init();

  initConsole();
  
  // Use of the faultHandling api itself...

  // 1: a buffer to hold the dump and the function to be called to process it
  faultHandlingSetDumpProcessor( faultDumpBuffer, consoleDumpProcessor );

  // 2: stack search parameters
  extern uint32_t __etext;
  extern uint32_t __StackTop;
   
  faultHandlingSetCallStackParameters( 0, &__etext, &__StackTop, 0 );

  // 3: what to do once the fauly has occurred: loop, reboot, etc
  faultHandlingSetPostFaultAction( POSTHANDLER_LOOP );

  consoleWrite( "Foo\r\n" );

  foo();

  consoleWrite( "Done\r\n" );
  
  return 0;
}

static void bar(void) {
}


static void foo(void) {

  /* 
	 By having local variables AND being a non-leaf function, we
	 ensure that our prolog pushes caller's r7 and lr, since we have
	 our own use for them.
  */
  uint32_t a[1];

  bar();

  /*
	Since stack frames are 8 byte aligned, then local vars, such as
	'a' here, can have padding.  Such padding will widen the gap
	between valid array indices and the prolog-pushed values of r7, lr
	(if both pushed). 

	Register pushes are from HIGHEST-NUMBERED to LOWEST-NUMBERED, so
	lr (r14) would be pushed FIRST, and r7 pushed SECOND.  In terms of
	accessing outside an array, if a[i] trashes r7, then a[i+1] will
	trash lr. The actual value of i is not well-defined (depends on
	stack frame alignment).
  */

  a[0] = 0xCAFEBABE;
  a[1] = 0xDEADBEEF;
  a[2] = 0xCAFEBABE;
  a[3] = 0xDEADBEEF;

}

/*
  We MUST define a HardFault_Handler.  It just vectors to
  faultHandling's provided FaultHandler.  This overrides the weak
  version in startup_efm32gg.c.

  The 'naked' attribute ensures that this function has no
  prolog/epilog that affect the stack (e.g. push r7,lr).
*/
__attribute__((naked))
void HardFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

// eof
