/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2017 Hellwig Geisse
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ptrace.h -- defines how registeres are laid out on the stack
 */

#ifndef __ASM_ECO32_PTRACE_H
#define __ASM_ECO32_PTRACE_H

#include <uapi/asm/ptrace.h>

#ifndef __ASSEMBLY__

/*
 * This struct describes how the registers are laid out on the
 * kernel stack during a syscall or other kernel entry.
 */

struct pt_regs {
	union {
		/* addressing a register with its numerical name */
		struct {
			unsigned long r0;
			unsigned long r1;
			unsigned long r2;
			unsigned long r3;
			unsigned long r4;
			unsigned long r5;
			unsigned long r6;
			unsigned long r7;
			unsigned long r8;
			unsigned long r9;
			unsigned long r10;
			unsigned long r11;
			unsigned long r12;
			unsigned long r13;
			unsigned long r14;
			unsigned long r15;
			unsigned long r16;
			unsigned long r17;
			unsigned long r18;
			unsigned long r19;
			unsigned long r20;
			unsigned long r21;
			unsigned long r22;
			unsigned long r23;
			unsigned long r24;
			unsigned long r25;
			unsigned long r26;
			unsigned long r27;
			unsigned long r28;
			unsigned long r29;
			unsigned long r30;
			unsigned long r31;
		};
		/* addressing a register with its functional name */
		struct {
			unsigned long zero;	/* zero by hardware */
			unsigned long ast;	/* reserved for assembler */
			unsigned long rv;		/* function return value */
			unsigned long scp;	/* static chain pointer */
			unsigned long arg_0;	/* function arguments */
			unsigned long arg_1;
			unsigned long arg_2;
			unsigned long arg_3;
			unsigned long tmp_0;	/* temporary variables, caller-save */
			unsigned long tmp_1;
			unsigned long tmp_2;
			unsigned long tmp_3;
			unsigned long tmp_4;
			unsigned long tmp_5;
			unsigned long tmp_6;
			unsigned long tmp_7;
			unsigned long loc_0;	/* local variables, callee-save */
			unsigned long loc_1;
			unsigned long loc_2;
			unsigned long loc_3;
			unsigned long loc_4;
			unsigned long loc_5;
			unsigned long loc_6;
			unsigned long loc_7;
			unsigned long os_0;	/* reserved for OS */
			unsigned long os_1;
			unsigned long os_2;
			unsigned long tp;		/* thread pointer */
			unsigned long fp;		/* frame pointer */
			unsigned long sp;		/* stack pointer */
			unsigned long xa;		/* exception return address */
			unsigned long ra;		/* function return address */
		};
		/* addressing a register with its numerical index */
		struct {
			unsigned long gpr[32];
		};
	};
	unsigned long r2_orig;	/* copy of r2, for signal handling/syscalls
				   (r2 may be changed and its original value
				   is needed later) */
	unsigned long psw;		/* processor status word */
};

#endif /* __ASSEMBLY__ */

#define user_stack_pointer(regs)	((unsigned long)((regs)->sp))
#define instruction_pointer(regs)	((unsigned long)((regs)->xa))
#define user_mode(regs)			(((regs)->psw & (1 << 25)) != 0)

#define r0_OFF		(0*4)
#define r1_OFF		(1*4)
#define r2_OFF		(2*4)
#define r3_OFF		(3*4)
#define r4_OFF		(4*4)
#define r5_OFF		(5*4)
#define r6_OFF		(6*4)
#define r7_OFF		(7*4)
#define r8_OFF		(8*4)
#define r9_OFF		(9*4)
#define r10_OFF		(10*4)
#define r11_OFF		(11*4)
#define r12_OFF		(12*4)
#define r13_OFF		(13*4)
#define r14_OFF		(14*4)
#define r15_OFF		(15*4)
#define r16_OFF		(16*4)
#define r17_OFF		(17*4)
#define r18_OFF		(18*4)
#define r19_OFF		(19*4)
#define r20_OFF		(20*4)
#define r21_OFF		(21*4)
#define r22_OFF		(22*4)
#define r23_OFF		(23*4)
#define r24_OFF		(24*4)
#define r25_OFF		(25*4)
#define r26_OFF		(26*4)
#define r27_OFF		(27*4)
#define r28_OFF		(28*4)
#define r29_OFF		(29*4)
#define r30_OFF		(30*4)
#define r31_OFF		(31*4)
#define r2_orig_OFF	(32*4)
#define psw_OFF		(33*4)

#endif /* __ASM_ECO32_PTRACE_H */
