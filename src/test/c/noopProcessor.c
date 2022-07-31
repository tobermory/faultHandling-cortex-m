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
#include "faultHandling.h"

/**
 * @author Stuart Maclean
 *
 * A program that uses the faultHandling api, so that if/when a fault occurs,
 * we are supplied a dump of the CPU state at fault time.
 *
 * Because this program is buildable for the generic 'ARM Cortex M' cpu,
 * with NO peripherals defined, we cannot actually export the fault dump
 * in any way (SD card, console uart, etc).
 *
 * See e.g. SiliconLabs test cases for fault dump processors that CAN
 * actually export the fault dump.
 */

/* We define the memory for the faultHandling api to write the dump to */
static char faultDumpBuffer[FAULT_HANDLING_DUMP_SIZE];

/* We define what to DO with the fault dump, in this case, nothing */
static void noopDumpProcessor(void) {
  
  // Wot, no periperhals to send the faultDump to, not even a serial port!

}

int main(void) {

  /* The faultHandling api */
  faultHandlingSetDumpProcessor( faultDumpBuffer, noopDumpProcessor );
  faultHandlingSetPostFaultAction( POSTHANDLER_LOOP );

  /*
	Force a fault by calling through a func pointer to a non-mapped address.
  */

  // step 1: The set up...
  void (*p)(void) = (void(*)(void))0x87654321;

  /*
	... and step 2, the failure.  This will cause our faultHandler to
	compose a fault dump in the app-supplied char[], followed by a call to
	the app-supplied dump processor.
  */
  p();
  
  return 0;
}

/*
  We MUST define a HardFault_Handler. It just vectors to faultHandling's
  provided FaultHandler. 

  The 'naked' attribute ensures that this function has no
  prolog/epilog that affect the stack (e.g. push r7,lr).
*/
__attribute__((naked))
void HardFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

#if 0
/*
  If we wanted to catch more-specific faults, e.g. MemManage faults,
  we would define this also:
*/

__attribute__((naked))
void MemManageFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

/*
  Such a program would need this too, to enable the MemManageFault_Handler:
*/

	SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

#endif
  
// eof
