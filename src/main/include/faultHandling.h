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
#ifndef CORTEXM_FAULT_HANDLING_H
#define CORTEXM_FAULT_HANDLING_H

#include <stdint.h>

/**
   Define CMSIS_device_header in your build tool/IDE. For example,
   in a Makefile, you'd do e.g.

   CPPFLAGS += -DCMSIS_device_header = \"myDevice.h\"
   
   We did not invent this approach.  ARM use it for their RTOS2/RTX
   builds.
*/
#include CMSIS_device_header

/**
 * @author Stuart Maclean
 *
 * Structured fault handling on the Cortex-M processor.  See
 * faultHandling.c for more details.
 *
 * @see faultHandling.c
 */

/**
 * What to do after the fault dump is populated and sent to the dump
 * processor.
 */
typedef enum { POSTHANDLER_LOOP,
			   POSTHANDLER_RESET,
			   POSTHANDLER_DEBUG,
			   POSTHANDLER_RETURN } faultHandlingPostFaultAction;


/**
 * This next typedef for documentation purposes only.  Currently, we
 * have no use for this type.  It lists the registers included in
 * the dump, for CM0, CM3/4.
 */
typedef struct {
  uint32_t r7;	// aka frame-pointer fp
  uint32_t sp;
  uint32_t excrt;
#if (__CORTEX_M > 0)
  uint32_t hfsr;
  uint32_t cfsr;
  uint32_t mmfar;
  uint32_t bfar;
#endif
  uint32_t shcsr;

  // The 8 stacked registers, always pushed to MSP/PSP on fault.
  uint32_t stkr0;
  uint32_t stkr1;
  uint32_t stkr2;
  uint32_t stkr3;
  uint32_t stkr12;
  uint32_t stklr;
  uint32_t stkpc;
  uint32_t stkpsr;
  
} faultHandlingRegSet;

/*
  An enum of the registers we are dumping. Note how the final element,
  FAULTHANDLING_CPUREG_COUNT, gives us the count we need in the processing.
*/
typedef enum { R7=0,
			   SP,
			   EXCRT,
			   PSR,
#if (__CORTEX_M > 0)
			   HFSR,
			   CFSR,
			   MMFAR,
			   BFAR,
#endif
			   SHCSR,
			   STKR0,
			   STKR1,
			   STKR2,
			   STKR3,
			   STKR12,
			   STKLR,
			   STKPC,
			   STKPSR,
			   FAULT_HANDLING_CPUREG_COUNT } faultHandlingRegIndex;

#define FAULT_HANDLING_CALLSTACK_ENTRIES (4)

/*
  The formatted fault dump (see faultHandling.c) has N 15-byte
  records, for the N regs above, then 4 18-byte records for call stack
  info, and a final NULL byte at end. Total size 328 bytes (CM3), which
  just fits in an Iridium SBD message!

  We define the size, 'FAULT_HANDLING_DUMP_SIZE', here so that
  application 'dump processors' can allocate a char[] of correct
  length to hold such a dump, e.g.

  char dumpBuffer[FAULT_HANDLING_DUMP_SIZE];
*/

/*
  A formatted CPU reg value 'row' is 

  label/5 + space/1 + value/8 + eol/1
*/
#define FAULT_HANDLING_CPUREG_ROWSIZE    (15)

/*
  A formatted 'pushed LR' value 'row' is

  addr/8 + space/1 + value/8 + eol/1
*/
#define FAULT_HANDLING_CALLSTACK_ROWSIZE (18)

/*
  The complete fault dump is then the cpu reg rows and the pushed LR
  rows, plus a trailing NULL byte (making the dump a valid C string).
*/
#define FAULT_HANDLING_DUMP_SIZE (FAULT_HANDLING_CPUREG_COUNT*\
								  FAULT_HANDLING_CPUREG_ROWSIZE+\
								  FAULT_HANDLING_CALLSTACK_ENTRIES*\
								  FAULT_HANDLING_CALLSTACK_ROWSIZE+1)

typedef void(*faultHandlingDumpProcessor)(void);

/**
 * Install the fault dump processor.  It is called if/when a fault occurs.
 */
void faultHandlingSetDumpProcessor( char* dumpBuffer,
									faultHandlingDumpProcessor dumpProcessor );

/**
 * Set the bounds for the 'pushed LR' search, i.e. the 'function call
 * stack'. 
 *
 * Any pushed LR will lie in the bounds of 'text_start' to 'text_end',
 * use these if your linker script defines them.  If no text_start
 * symbol, approximate that with __Vectors_End or even __Vectors..  If
 * that not available either, just use 4, or the offset for which the
 * application was built (non-zero if bootloader present). Do NOT use
 * 0.
 *
 * These bounds for text (code) will be incorrect if you have
 * RAMFUNCS, and the search will miss (not identify) pushed LRs in
 * this case.
 *
 * @param textLo - lowest address that could contain code, must be > 0.
 *
 * @param textHi - highest address that could contain code.
 *
 * @param mspTop - top of main stack.  Likely there is a
 * linker-script-defined variable that the application can use for
 * this parameter, for GNU .ld files this is typically '__StackTop'.
 *
 * @param pspTop - top of process stack.  Application should pass 0 if
 * not using the process stack.  0 can be passed also if the
 * application just doesn't know a good value for top of process
 * stack.  We will just use mspTop.
 *
 * Only threads, under an RTOS, would use the process stack. Not
 * trivial to locate the top of this 'thread stacks' area.  Address of
 * '__bss_end__' (GNU linker scripts) is a pessimistic approximation,
 * but still better (i.e. lower address) than mspTop.
 */

void faultHandlingSetCallStackParameters( uint32_t* textLo,
										  uint32_t* textHi,
										  uint32_t* mspTop,
										  uint32_t* pspTop );

/**
 * Set what to do after the fault dump is delivered to the dump processor.
 */
void faultHandlingSetPostFaultAction( faultHandlingPostFaultAction );

/**
   Needed by application fault handlers (asm), e.g.

__attribute__((naked))
void HardFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

It MUST be a simple branch (B), i.e. a jump, and NOT a branch+link
(BL), i.e. NOT a call.
*/

void FaultHandler(void);

#endif

// eof
