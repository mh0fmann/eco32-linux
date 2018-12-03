/*
 * Linux architectural port borrowing liberally from similar works of
 * others, namely OpenRISC and RISC-V.  All original copyrights apply
 * as per the original source declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Hellwig Geisse
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/unistd.h>
#include <linux/audit.h>
#include <linux/tracehook.h>

#include <asm/ptrace.h>
#include <asm/syscalls.h>
#include <asm/thread_info.h>
#include <asm/irqflags.h>


#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (sys_call_ptr_t) (call),


/* syscall table */
sys_call_ptr_t syscall_table[__NR_syscalls] = {
    [0 ... __NR_syscalls-1] = (sys_call_ptr_t) sys_ni_syscall,
#include <asm/unistd.h>
};


/*
 * ISR for syscalls
 */
void ISR_syscall(int irq, struct pt_regs* regs)
{
    unsigned int num;
    unsigned int res;

    /* syscalls run with interrupts enabled */
    local_irq_enable();

    /* skip over trap instruction */
    regs->r30 += 4;
    /* check for legal syscall number */
    num = regs->r2;

    if (num >= __NR_syscalls) {
        regs->r2 = sys_ni_syscall();
        return;
    }

    syscall_trace_enter(regs);

    /* call syscall handling function */
    res = (*syscall_table[num])(regs->r4, regs->r5, regs->r6,
                                regs->r7, regs->r8, regs->r9);
    regs->r2 = res;

    syscall_trace_leave(regs);
}
