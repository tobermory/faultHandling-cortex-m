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
 * Fault capture using the faultHandling api, where the processing of
 * the fault is to 'export the fault dump' by writing it to a uart on
 * the SiliconLabs STK3700 starter kit.  The STK3700 uses an
 * EFM32GG990F1024 cpu.
 *
 * We are viewing that uart as the 'serial console' of that board, so
 * the dump export is as close to a 'printf' as such a board can
 * mimic. 

 * On the STK3700, Expansion Header pin 4 (USART1 Tx) is PD0 while pin
 * 6 (USART1 Rx) is PD1. These pins are route/location 1 for that
 * USART.  See the STK3700 data sheet.

 * Hook up a ttl-usb cable to your host, and open e.g. minicom (or
 * just cat!) at 115200 and see the fault dump appear as the stk3700
 * end keels over with a fault!
 */

void initConsole(void);
void consoleWrite( char* s );

// A place to hold the formatted fault dump, of correct size.
static char faultDumpBuffer[FAULT_HANDLING_DUMP_SIZE];

/**
 * Dump the fault to the serial console, so a user can 'see' what went wrong.
 */
void consoleDumpProcessor(void) {
  consoleWrite( faultDumpBuffer );
}

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

  /*
	Force a invState fault by calling through a zeroed func pointer.
  */

  // The set up...
  void (*p)(void) = (void(*)(void)) 0x20202020;

  // ... and the failure
  p();
  
  return 0;
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
