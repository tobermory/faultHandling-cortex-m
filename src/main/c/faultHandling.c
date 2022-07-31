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
#include <string.h>

/**
 * @author Stuart Maclean

  Project-local headers we snarfed from
  CMSIS_5/Device/ARM/ARMCMX/Include/*.h.  We need this for the
  definition of the SCB_Type struct and the SCB pointer,
  e.g. SCB->HFSR, etc. Contents of SCB vary by cm0, cm3, cm4.

  Each of these files includes the respective core_cmN.h. In each of
  those headers, __CORTEX_M is defined to be 0, 3, 4, etc.  We'll use
  that define below, it's more standard than our own build process
  defines: __CORTEX_M3, etc.  But our build process, e.g. make, must
  supply e.g. __CORTEX_M3, __CORTEX_M0PLUS, etc.
*/
#if defined __CORTEX_M0PLUS
#include "ARMCM0plus.h"
#elif defined __CORTEX_M3
#include "ARMCM3.h"
#elif defined __CORTEX_M4
#include "ARMCM4.h"
#else
#error "Cortex Platform Not Specified"
#endif

#include "faultHandling.h"

/**
 * @author Stuart Maclean
 *
 * Structured fault handling on the Cortex-M processor.  
 *
 * When a fault occurs (HardFault or any of MemManage, BusFault,
 * UsageFault), our fault handler collects processor state, formats
 * that state into a 'fault dump table' in a human readable format,
 * then offers it to a pre-registered 'dump processor' function.  

 * The formatted 'fault dump table' looks thus. First N lines are each
 * label+space+hexvalue, for N lines each holding one CPU
 * register. Following those is an optional call stack inference.  On
 * CM3, a dump table looks like this:

r7    2001FFF0
sp    2001FFD0
excrt FFFFFFF9
psr   20000003
hfsr  40000000
cfsr  00020000
mmfar E000ED34
bfar  E000ED38
shcsr 00000000
s.r0  00000002
s.r1  0000000A
s.r2  20000A3C
s.r3  00000000
s.r12 20000B38
s.lr  000001AF
s.pc  00000000
s.psr 40000000
20000FE4 00000317
20000FEC 000002ED
20000FF4 000002AF
20000FFC 00000127

* The call-stack pairs, of which there are 4 above, indicate where in
* the ram the pushed LR was found, and the LR value itself.  Combine
* these values with the .map file for your faulting program (you do
* HAVE the .map file right?) to derive the function call stack.

 * Some of the registers above are missing on CM0/0+ (no hfsr, etc).

 * The dump table is passed to the processor. The processor's job is
 * to decide what to DO with the dump:

 * Print to a 'console'
 * Saved to 'disk' 
 * Stream to an overhead satellite (welcome to MY world)

 * After the processor is done, we take a course of action according
 * to caller's setup: return, loop, reset/reboot or
 * set-debug-breakpoint.
 *
 * To use this faultHandling api in an application, be sure to include
 * at LEAST the HardFault_Handler (given below) and optionally any
 * desired combo of MemManage, BusFault and UsageFault handlers (CM3/4).
 * These functions go in the program's Vector table, and override weak
 * versions supplied by the startup code.
 *
 * Then, to use this api, just call 3 routines early in main:
 *
 * 1 reserve a buffer and set a dump processor:
 *
 * char buf[FAULT_HANDLING_DUMP_SIZE];
 * faultHandlingSetDumpProcessor( buf, myProcessor );
 *
 * 2 (optional), if you want the fault handler to infer a call stack
 * leading up to the fault, supply .text section boundaries, an upper
 * limit on MSP (likely top-of-ram) and an upper limit of PSP
 * (if using rtos, if not, pass 0):

 * faultHandlingSetCallStackParameters( &Vectors, &__etext, &__StackTop, 0 );
 *
 * The symbols Vectors, __etext, __StackTop are likely defined by your linker
 * script, and can be used in code.
 *
 * 3 finally, state what you want to occur at fault time: spin, reboot, return:
 *
 * faultHandlingSetPostFaultAction( REBOOT );
 *
 * Now just wait for your program to blow up!

 * The dump processor is passed the 'fault table' at time of fault.
 * Processor might write to serial console, swo, to disk (?) or even
 * to RAM. In my apps, I have to beam the fault dump over Iridium SBD
 * from a remotely-deployed instrument!
 *
 * For examples, see swoTest.c, consoleTest.c, others in src/test/c
 * 
 * We draw HEAVILY on Chap 12 of Yiu's 3rd ed of Cortex M3/M4
 * Definitive Guide, which should be considered a MUST-READ ;)
 *
 * Why include r7 in the dump?  Well, the usual C function prolog (on
 * function entry) is 'push {r7,lr}', so that the function can itself
 * update these two regs and still get back to caller, via the usual
 * epilogs: 'pop {r7, lr}; bx lr' or the shorter 'pop {r7, pc}'.

 * If our fault is a bad PC, it may be due to the epilog encountering
 * a trashed stack location for the pushed lr.  If we trashed the
 * pushed lr, we may have also trashed the adjacent pushed r7.  If
 * fault dump values for r7, pc are same/related, it lends weight to
 * the notion that the fault is indeed due to stack smashing:

 int stackLocalVar[2];

 stackLocalVar[2] = X;
 stackLocalVar[3] = Y;

 The two erroneous array accesses above may trash the prolog-pushed
 r7, lr, causing the fault to occur at function return (epilog). If r7
 shows X and pc (loaded from lr) shows Y, good bet stack corruption
 occurred.

 @see faultGuru.c for a tool that attempts to make sense of the fault
 table output.
 */

/* Total RAM space needed by fault handler api */
static char* dumpBuffer = NULL;
static faultHandlingDumpProcessor dumpProcessor = NULL;
static uint32_t startText, endText, mspTop, pspTop;
static faultHandlingPostFaultAction postFaultAction = POSTHANDLER_LOOP;

static void faultDumpPrepare(void);
static void formatRegValue( faultHandlingRegIndex index, uint32_t value );
static void formatCallStackPair( int index, uint32_t addr, uint32_t val );

/*
  Our formatted 'fault dump table' of the N registers we are dumping
  is made up of 'label plus value', for each cpu register, plus some
  line-endings for pretty-printing purposes.

  Here we set up the entire multi-line string, with blanks for the reg
  values. Then, at fault time, we just 'fill in the holes' with
  what-went-wrong. A short header and footer markup the formatted
  text.
*/

void faultHandlingSetDumpProcessor( char* buf, faultHandlingDumpProcessor p ) {
  dumpBuffer = buf;
  dumpProcessor = p;
  faultDumpPrepare();
}

/**
 * @param mspTop - Top of main stack, likely top of RAM.
 *
 * @param pspTop - only needed if running threads in an RTOS.
 * Otherwise, set to 0.
 */
void faultHandlingSetCallStackParameters( uint32_t* textLo,
										  uint32_t* textHi,
										  uint32_t* mspTop_,
										  uint32_t* pspTop_ ) {
  startText = (uint32_t)textLo;
  endText = (uint32_t)textHi;
  mspTop = (uint32_t)mspTop_;
  pspTop = pspTop_ == 0 ? mspTop : (uint32_t)pspTop_;
}

void faultHandlingSetPostFaultAction( faultHandlingPostFaultAction pfa ) {
  postFaultAction = pfa;
}

/**
 * As per Yiu 3rd Ed, p 401. Other page numbers below refer to same text.
 *
 * @param r7 - frame pointer at time of fault (useful for 'pop
 * {r7,pc}' diagnosis, see above commentary?)
 *
 * @param stack - where the 8 stacked regs are found (is sp).
 *
 * @param excrt - exception return value in LR at time of fault.
 *
 * We've chosen to order the parameters passed by increasing reg
 * number: r7, r13 (sp), r14 (lr, which is excRet upon fault). This
 * ordering is arbitrary, but accommodates should we ever want to add
 * more regs.
 */
void FaultHandler_C( uint32_t r7, uint32_t* stack, uint32_t excRet ) {

  // NOT set up correctly if we have no processor!
  if( !dumpProcessor )
	return;
  
  /*
	Vital SCB registers, give clues to the fault cause.  SCB
	contents differ across CM platforms.
  */

#if (__CORTEX_M > 0)
  uint32_t hfsr  = SCB->HFSR;
  uint32_t cfsr  = SCB->CFSR;
  uint32_t bfar  = SCB->BFAR;
  uint32_t mmfar = SCB->MMFAR;
#endif
  
  // see p 264, indicates enabled handlers at time of fault
  uint32_t shcsr = SCB->SHCSR;

  /*
	On Cortex M (0,3,4), eight regs are stacked, see p 394.  This is
	the (partial) state of the running program when the fault occured.
  */
  uint32_t sp = (uint32_t)stack;
  uint32_t r0 = stack[0];
  uint32_t r1 = stack[1];
  uint32_t r2 = stack[2];
  uint32_t r3 = stack[3];
  uint32_t r12 = stack[4];
  uint32_t lr = stack[5];
  uint32_t pc = stack[6];
  uint32_t psr = stack[7];

  /*
	Current psr, contains IPSR [8..0] which tells us the active fault
	handler(hard, usage, etc).  Always 3=HardFault on CM0/0+.
  */
  uint32_t psrNow = __get_xPSR();

  // For EXC_RETURN decoding, see p 278, and below
  formatRegValue( R7, r7 );
  formatRegValue( SP, sp );
  formatRegValue( EXCRT, excRet );
  formatRegValue( PSR, psrNow );

#if (__CORTEX_M > 0)
  formatRegValue( HFSR, hfsr );
  formatRegValue( CFSR, cfsr );
  /*
	It is up to dump analyzer (e.g. our faultGuru.c) to determine, via
	cfsr bit masks, whether mmfar, bfar reg are valid.  Our job is
	just to make them available!
  */
  formatRegValue( MMFAR, mmfar );
  formatRegValue( BFAR, bfar );
#endif
  
  formatRegValue( SHCSR, shcsr );
  
  formatRegValue( STKR0, r0 );
  formatRegValue( STKR1, r1 );
  formatRegValue( STKR2, r2 );
  formatRegValue( STKR3, r3 );
  formatRegValue( STKR12, r12 );
  formatRegValue( STKLR, lr );
  formatRegValue( STKPC, pc );
  formatRegValue( STKPSR, psr );



  /*
	Heuristics to locate the function call stack leading up to the
	fault.  We basically search the stack (starting at the addr above
	the stacked regs) until we've found N values that may be pushed LR
	regs, or until we reach some TopOfStack limit.

	In an application w RTOS, we'd likely have threads that use their
	own Process stack.  In that case, would be better to terminate the
	search when see an LR = osThreadExit, rather than when hitting
	__StackTop (which would likely be a LONG way from a Process
	Stack). IDEA: can test LR to see if Process Stack is the one we are
	searching, same way that asm code did to LOCATE that stack.
  */
  if( endText > 0 ) {
	int found = 0;
	
	/*
	  8 regs are stacked prior to fault handler entry, so start 
	  the 'pushed LR's search above those.

	  Depending on the stack in use at time of fault, we use
	  the mspTop or pspTop sentinel to bound the stack search.
	*/
	uint32_t TOS = excRet & 4 ? pspTop : mspTop;
	
	for( uint32_t* fp = stack + 8; fp < (uint32_t*)TOS; fp++ ) {
	  
	  uint32_t val = *fp;
	
	  /*
		Code section is bounded by these two addresses. Any LR would
		be within that range.
	  */
	  if( val < (uint32_t)startText || val > (uint32_t)endText )
		continue;
	
	  // On M3, pc[0] == 1, so any LR must have this property too.
	  if( (val & 1) == 0 )
		continue;

	  // Deem that this word is indeed a 'pushed LR'.
	  formatCallStackPair( found, (uint32_t)fp, val );

	  // Found as many as we want, or have ROOM for in the dump table ?
	  found++;
	  if( found == FAULT_HANDLING_CALLSTACK_ENTRIES )
		break;
	}
  }
  
  // The fault table is now complete, ship it out the door!
  dumpProcessor();

  // Once the fault packaged up and offered to processor, what do we do next?
  switch( postFaultAction ) {

  case POSTHANDLER_LOOP:
	while(2)
	  ;
	break;

  case POSTHANDLER_RESET:
	NVIC_SystemReset();
	break;

  case POSTHANDLER_DEBUG:
#define DEBUG_BREAK           __asm__("BKPT #0")
	DEBUG_BREAK;
	break;

  case POSTHANDLER_RETURN:
	break;
	
  default:
	;
  }
}

						 

/************************ STATICS, PRIVATE IMPLEMENTATION *****************/

static const char* const cpuRegLabels[] =
  { "r7   ",
	"sp   ",
	"excrt",
	"psr  ",
#if (__CORTEX_M > 0)
	"hfsr ",
	"cfsr ",
	"mmfar",
	"bfar ",
#endif
	"shcsr",
	"s.r0 ",
	"s.r1 ",
	"s.r2 ",
	"s.r3 ",
	"s.r12",
	"s.lr ",
	"s.pc ",
	"s.psr"
  };

static void faultDumpPrepare(void) {

  int cursor = 0;

  // N cpu registers
  for( int i = 0; i < FAULT_HANDLING_CPUREG_COUNT; i++ ) {
	// Each line of output is '5-char-LABEL 8-char-VALUE\r\n' = 16 chars

	// The label and space char: 0..5
	strcpy( dumpBuffer + cursor, cpuRegLabels[i] );
	dumpBuffer[cursor+5]  = ' ';
	
	// The hole for hex-formatted reg value is [6]..[13]

	// The eol
	dumpBuffer[cursor+14] = '\n';
	//	faultTable[cursor+15] = '\n';

	// next 'row'
	cursor += FAULT_HANDLING_CPUREG_ROWSIZE;
  }

  /*
	Call stack leading to the fault. Each line is '8-char-ADDR
	8-char-VALUE\n' = 18 chars, 4 lines total.
  */
  for( int i = 0; i < FAULT_HANDLING_CALLSTACK_ENTRIES; i++ ) {
	dumpBuffer[cursor+8] = ' ';
	dumpBuffer[cursor+17] = '\n';
	formatCallStackPair( i, 0, 0 );
	cursor += FAULT_HANDLING_CALLSTACK_ROWSIZE;
  }
  
  // Trailing NULL, final byte in the fault dump.
  dumpBuffer[cursor] = 0;
}
  


/*
  Overkill to call sprintf when we have ONE value we know we want
  HEX formatted, so do it locally, a nibble at a time ;)
*/
static char hex[16] = { '0', '1', '2', '3',
						'4', '5', '6', '7',
						'8', '9', 'A', 'B',
						'C', 'D', 'E', 'F' };

/**
 * Format one register value into the fault table string
 */
static void formatRegValue( faultHandlingRegIndex index, uint32_t value ) {
  
  /*
	Locate the 8-char hole for this reg value in the fault dump
	string, index identifies the 'row'.
  */
  int cursor = 15*index + 6;

  /* and fill it in with hex-encoded reg value */
  for( int i = 0; i < 8; i++ ) {
	uint32_t nibble = (value >> (28-4*i)) & 0xf;
	dumpBuffer[cursor+i] = hex[nibble];
  }
}

static void formatCallStackPair( int index, uint32_t addr, uint32_t val ) {
  
  /*
	Locate the 2 8-char holes in the fault table string part reserved for 
	call stack info, index identifies the 'row'.
  */
  int cursor = FAULT_HANDLING_CPUREG_ROWSIZE*FAULT_HANDLING_CPUREG_COUNT +
	FAULT_HANDLING_CALLSTACK_ROWSIZE*index;

  /* and fill it in with hex-encoded reg values */
  for( int i = 0; i < 8; i++ ) {
	uint32_t nibbleA = (addr >> (28-4*i)) & 0xf;
	uint32_t nibbleV = (val  >> (28-4*i)) & 0xf;
	dumpBuffer[cursor+i] = hex[nibbleA];
	dumpBuffer[cursor+9+i] = hex[nibbleV];
  }
}

/************************ END PRIVATE IMPLEMENTATION *****************/

/**
 * We include FOUR fault handlers for reference here. ALL vector to
 * our asm FaultHandler above.

 * The HardFault_Handler MUST be copied to an application source file.
 * It will NOT be picked up by the linker if ONLY located in here in
 * libfaultHandling.a!
 *
 * The other three handlers are useful if you want to know the EXACT
 * fault type. Without these, the fault dump will show psr[8..0] = 3
 * (HardFault) and show 'escalated' fault (hfsr[30] = 1) but you won't
 * know WHICH fault was escalated. All three handlers vector
 * immediately to our FaultHandler.
 *
 * To enable these faults also require application set-up:
 *
 * SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;       // MemManage faults
 * 
 * SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;       // BusFault faults
 *
 * SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;       // UsageFault faults
 * 
 * If you enable any of the three faults as above, you MUST copy in to
 * your application the corresponding handler below.  Otherwise,
 * you'll get the startup code's DefaultHandler, which is often
 * while-1, and none of the above fault handling will occur.
 */

#if 0

__attribute__((naked))
void HardFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

__attribute__((naked))
void MemManage_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

__attribute__((naked))
void BusFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

__attribute__((naked))
void UsageFault_Handler(void) {
  __asm__( "B FaultHandler\n" );
}

#endif

// eof
