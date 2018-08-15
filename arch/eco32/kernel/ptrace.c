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
 * ptrace.c -- ??
 */

#include <linux/elf.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/regset.h>
#include <linux/smp.h>
#include <linux/audit.h>
#include <linux/tracehook.h>

#include <asm/page.h>
#include <asm/uaccess.h>

long arch_ptrace(struct task_struct* child, long request,
                 unsigned long addr, unsigned long data)
{
	panic("function %s in file %s not implemented",
	      __FUNCTION__, __FILE__);
	return 0;
}

void ptrace_disable(struct task_struct* child)
{
	panic("function %s in file %s not implemented",
	      __FUNCTION__, __FILE__);
}

const struct user_regset_view* task_user_regset_view(struct task_struct* task)
{
	panic("function %s in file %s not implemented",
	      __FUNCTION__, __FILE__);
	return NULL;
}

/*
 * Notification of system call entry/exit
 * - triggered by current->work.syscall_trace
 */
asmlinkage long do_syscall_trace_enter(struct pt_regs* regs)
{
	long ret = 0;

	if (test_thread_flag(TIF_SYSCALL_TRACE) &&
	    tracehook_report_syscall_entry(regs))
		/*
		 * Tracing decided this syscall should not happen.
		 * We'll return a bogus call number to get an ENOSYS
		 * error, but leave the original number in <something>.
		 */
		ret = -1L;

	audit_syscall_entry(regs->gpr[11], regs->gpr[3], regs->gpr[4],
	                    regs->gpr[5], regs->gpr[6]);

	return ret ? : regs->gpr[11];
}

asmlinkage void do_syscall_trace_leave(struct pt_regs* regs)
{
	int step;

	audit_syscall_exit(regs);

	step = test_thread_flag(TIF_SINGLESTEP);

	if (step || test_thread_flag(TIF_SYSCALL_TRACE))
		tracehook_report_syscall_exit(regs, step);
}
