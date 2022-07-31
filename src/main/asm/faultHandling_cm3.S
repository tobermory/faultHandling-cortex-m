/*
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
	// Called by HardFault_Handler, on CM3/4 platforms, when
	// our 'structured fault handler' api is in use.

	.file "faultHandling_cm3.S"
	.syntax unified

	.thumb
    .section ".text"
    .align   2

	.thumb_func
    .type    FaultHandler, %function
    .global  FaultHandler
    .fnstart
    .cantunwind
FaultHandler:
	
	// In order to call FaultHandler_C with r7, r13 (sp) and r14 (lr)
	// as parameters, in that order, we have to get those values
	// into r0, r1 and r2 respectively. sp may be MSP or PSP, which
	// is revealed by bit 2 of LR (EXC_RETURN).

	TST LR, #4
	ITE EQ		
	MRSEQ R1, MSP
	MRSNE R1, PSP
	MOV R2, LR
	MOV R0, R7
	B FaultHandler_C

	.fnend
    .size    FaultHandler, .-FaultHandler

	.end
	
