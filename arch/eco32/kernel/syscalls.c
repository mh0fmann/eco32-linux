/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Hellwig Geisse, Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * syscalls.c -- system calls
 */


#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/unistd.h>
#include <linux/audit.h>
#include <linux/tracehook.h>

#include <asm/ptrace.h>
#include <asm/syscalls.h>
#include <asm/thread_info.h>


/*
 * Notify kernel of syscall entry.
 */
// currently not used
//static noinline void syscall_trace_enter(struct pt_regs* regs)
//{
//	/* check if tracing allows this syscall */
//	if (test_thread_flag(TIF_SYSCALL_TRACE) &&
//	    tracehook_report_syscall_entry(regs)) {
//		/* it does not */
//		return;
//	}
//
//	audit_syscall_entry(regs->r2,
//	                    regs->r4, regs->r5,
//	                    regs->r6, regs->r7);
//}


/*
 * Notify kernel of syscall leave.
 */
//static noinline void syscall_trace_leave(struct pt_regs* regs)
//{
//	int step;
//
//	audit_syscall_exit(regs);
//	step = test_thread_flag(TIF_SINGLESTEP);
//
//	if (step || test_thread_flag(TIF_SYSCALL_TRACE)) {
//		tracehook_report_syscall_exit(regs, step);
//	}
//}


/*
 * syscall table
 */

#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (sys_call_ptr_t) (call),

sys_call_ptr_t syscall_table[__NR_syscalls] = {
	[0 ... __NR_syscalls-1] = (sys_call_ptr_t) sys_ni_syscall,
#include <asm/unistd.h>
};


/*
 * syscall interface:
 *   syscall # in $2
 *   args in $4..$9
 */
void ISR_syscall(int irq, struct pt_regs* regs)
{
	unsigned int num;
	unsigned int res;

	/* syscalls run with interrupts enabled! */
	__asm__("mvfs $8,0			\n"
	        "ldhi $9,0x00800000	\n"
	        "ori $9,$9,0x0000	\n"
	        "or $8,$8,$9		\n"
	        "mvts $8,0			\n"
	        : : : "$8", "$9");
	/* skip over trap instruction */
	regs->r30 += 4;
	/* check for legal syscall number */
	num = regs->r2;

	if (num >= __NR_syscalls) {
		regs->r2 = sys_ni_syscall();
		return;
	}

	/* call syscall handling function */
	res = (*syscall_table[num])(regs->r4, regs->r5, regs->r6,
	                            regs->r7, regs->r8, regs->r9);
	regs->r2 = res;
}
