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
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/kallsyms.h>
#include <asm/uaccess.h>

#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/irq.h>

#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/unistd.h>
#include <linux/audit.h>
#include <linux/tracehook.h>

#include <asm/ptrace.h>
#include <asm/syscalls.h>
#include <asm/thread_info.h>


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


static void sig_or_panic(int irq, struct pt_regs* regs, int signo)
{
    siginfo_t info;

    if (user_mode(regs)) {
        info.si_signo = signo;
        info.si_errno = 0;
        info.si_addr = (void*) regs->xa;
        force_sig_info(signo, &info, current);
    }else{
        def_xcpt_handler(irq, regs);
    }
}


void ISR_ill_inst(int irq, struct pt_regs* regs)
{
    sig_or_panic(irq, regs, SIGILL);
}


void ISR_prv_inst(int irq, struct pt_regs* regs)
{
    sig_or_panic(irq, regs, SIGILL);
}


void ISR_div_inst(int irq, struct pt_regs* regs)
{
    sig_or_panic(irq, regs, SIGFPE);
}


void ISR_ill_addr(int irq, struct pt_regs* regs)
{
    sig_or_panic(irq, regs, SIGBUS);
}


void ISR_prv_addr(int irq, struct pt_regs* regs)
{
    sig_or_panic(irq, regs, SIGSEGV);
}


void __init trap_init(void)
{
    set_ISR(XCPT_ILL_INST, ISR_ill_inst);
    set_ISR(XCPT_PRV_INST, ISR_prv_inst);
    set_ISR(XCPT_DIV_INST, ISR_div_inst);
    set_ISR(XCPT_ILL_ADDR, ISR_ill_addr);
    set_ISR(XCPT_PRV_ADDR, ISR_prv_addr);
    set_ISR(XCPT_TRAP_INST, ISR_syscall);
}
