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
	// Called by HardFault_Handler, on CM0/CM0+ platforms, when
	// our 'structured fault handler' api is in use.

	.file "faultHandling_cm0.S"
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

	// Unlike the cm3 version, cm0/0+ doesn't have IT instruction,
	// so everything a bit more manual here.
	MOV R0,LR
	LSRS R0,R0,#3
	BCC MRS_MSP
	MRS R1, PSP
	B POST_MRS
MRS_MSP:
	MRS R1, MSP	
POST_MRS:
	MOV R2, LR
	MOV R0, R7

	// Is it safe to just 'B FaultHandler_C'? The B range on CM0 is
	// only +-2046 bytes.  If this fails, the LDR+BX will work.

	// Will it 'fail' at build/link time, or runtime?

	// The validity of the B must depend on WHERE FaultHandler_C sits
	// relative to this code, in the final binary??
	// Since the two are built into the SAME .a file, with FaultHandler
	// preceding FaultHandler_C, the gap between
	// the two is only the handful of instructions above.
	
	//	LDR R3,=FaultHandler_C
	//	BX R3
	B FaultHandler_C

	.fnend
    .size FaultHandler, .-FaultHandler

	.end
	
