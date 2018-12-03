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


#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched/signal.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

#include <asm/irq.h>
#include <asm/ptrace.h>
#include <asm/unistd.h>
#include <asm/syscalls.h>


static char* exceptionCause[32] = {
    /* 00 */  "terminal 0 transmitter interrupt",
    /* 01 */  "terminal 0 receiver interrupt",
    /* 02 */  "terminal 1 transmitter interrupt",
    /* 03 */  "terminal 1 receiver interrupt",
    /* 04 */  "keyboard interrupt",
    /* 05 */  "unknown interrupt",
    /* 06 */  "unknown interrupt",
    /* 07 */  "unknown interrupt",
    /* 08 */  "disk interrupt",
    /* 09 */  "unknown interrupt",
    /* 10 */  "unknown interrupt",
    /* 11 */  "unknown interrupt",
    /* 12 */  "unknown interrupt",
    /* 13 */  "unknown interrupt",
    /* 14 */  "timer 0 interrupt",
    /* 15 */  "timer 1 interrupt",
    /* 16 */  "bus timeout exception",
    /* 17 */  "illegal instruction exception",
    /* 18 */  "privileged instruction exception",
    /* 19 */  "divide instruction exception",
    /* 20 */  "trap instruction exception",
    /* 21 */  "TLB miss exception",
    /* 22 */  "TLB write exception",
    /* 23 */  "TLB invalid exception",
    /* 24 */  "illegal address exception",
    /* 25 */  "privileged address exception",
    /* 26 */  "unknown exception",
    /* 27 */  "unknown exception",
    /* 28 */  "unknown exception",
    /* 29 */  "unknown exception",
    /* 30 */  "unknown exception",
    /* 31 */  "unknown exception"
};



void def_xcpt_handler(int irq, struct pt_regs* regs)
{
    panic("%s at 0x%08lx", exceptionCause[irq], regs->xa);
}


/*
 * All generlly speaking unwanted exceptions arrive here.
 * If we were not in kernel mode we can handle them with signals to
 * the process.
 * Otherwise there is no way out
 */
void do_exception(int irq, struct pt_regs* regs)
{
    int signo;
    siginfo_t info;

    if (user_mode(regs)) {
        switch (irq) {
            case XCPT_ILL_INST:
                signo = SIGILL;
                break;
            case XCPT_PRV_INST:
                signo = SIGILL;
                break;
            case XCPT_DIV_INST:
                signo = SIGFPE;
                break;
            case XCPT_ILL_ADDR:
                signo = SIGBUS;
                break;
            case XCPT_PRV_ADDR:
                signo = SIGSEGV;
                break;
            default:
                goto failed;
        }

        info.si_signo = signo;
        info.si_errno = 0;
        info.si_addr = (void*) regs ->xa;
        force_sig_info(signo, &info, current);
        return;
    }

failed:
    def_xcpt_handler(irq, regs);
}


/*
 * The syscalls requested with trap arrive here
 */
void do_trap(int irq, struct pt_regs* regs)
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



void __init trap_init(void)
{
    set_ISR(XCPT_ILL_INST, do_exception);
    set_ISR(XCPT_PRV_INST, do_exception);
    set_ISR(XCPT_DIV_INST, do_exception);
    set_ISR(XCPT_ILL_ADDR, do_exception);
    set_ISR(XCPT_PRV_ADDR, do_exception);
    set_ISR(XCPT_TRAP_INST, do_trap);
}
