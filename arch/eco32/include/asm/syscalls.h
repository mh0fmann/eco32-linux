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

#ifndef __ASM_ECO32_SYSCALLS_H
#define __ASM_ECO32_SYSCALLS_H

typedef long (*sys_call_ptr_t)(long arg0, long arg1, long arg2,
                               long arg3, long arg4, long arg5);

void ISR_syscall(int irq, struct pt_regs* regs);

/*
 * Calling conventions for the following system calls
 * can differ, so it's possible to override them.
 */

asmlinkage long sys_rt_sigreturn(struct pt_regs* regs);

#endif /* __ASM_ECO32_SYSCALLS_H */
