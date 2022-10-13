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

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

/**
 * @author Stuart Maclean
 *
 * Helper for fault handling api test cases that run on a SiliconLabs
 * stk3200 starter kit board.  That board contains a ZeroGecko CPU, a
 * CM0plus.

 * https://www.silabs.com/development-tools/mcu/32-bit/efm32zg-starter-kit?tab=overview 

 * Here we provide TWO routines for test cases:

 * initConsole() - set up the CMU, GPIO and USART peripherals such 
 * that USART1 is pinned out to the stk3200's Expansion Header on pins
 * 4 (Tx) and 6 (Rx). We are viewing that uart as the 'serial console'
 * of that board.

 * consoleWrite( char* s ) - write a supplied string to the serial console.
 *
 * Hook up a ttl-usb cable to your host, and open
 * e.g. minicom/teraterm (or just cat!) at 115200 baud, 8N1, and see
 * any output written by the stk3200's consoleWrite routine.
 *
 * $ stty -F /dev/ttyXXX 115200 -echo
 * $ cat /dev/ttyXXX
 */

/**
 * Prepare the stk3200's cmu, gpio and usart peripherals so that we
 * have a 'serial console' to which we can write a fault dump when one
 * occurs. This is all typical emlib stuff.
 */
void initConsole(void) {
  
  CMU_OscillatorEnable( cmuOsc_HFXO, true, true );
  CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );

  // USART1#2 = Tx - D7, Rx - D6
  CMU_ClockEnable( cmuClock_GPIO, true );
  GPIO_PinModeSet( gpioPortD, 7, gpioModePushPull, 1 );
  GPIO_PinModeSet( gpioPortD, 6, gpioModeInput, 0 );

  CMU_ClockEnable( cmuClock_USART1, true );

  // At 24MHz (HFXO), oversampling of 6 gives lowest baud rate error for 115200
  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
  init.oversampling = usartOVS6;
  init.enable = usartDisable;

  /*
	USART_InitAsync calls USART_Reset, which resets the ROUTE register.
	So, do the ROUTE setup AFTER the Init call.
  */
  USART_InitAsync( USART1, &init );
  USART1->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN |
	USART_ROUTE_LOCATION_LOC2;   

  USART_Enable( USART1, usartEnable );
}

/**
 * Write null-terminated string @p s to the stk3700 'serial console',
 * defined by us to be USART1.
 */
void consoleWrite( char* s ) {
  char* cp = s;
  while( *cp ) {
	USART_Tx( USART1, *cp );
	cp++;
  }
}

// eof
