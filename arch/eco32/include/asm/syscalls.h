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


typedef long (*sys_call_t)(long arg0, long arg1, long arg2,
                           long arg3, long arg4, long arg5);

extern sys_call_t syscall_table[];

/*
 * Calling conventions for the following system calls
 * can differ, so it's possible to override them.
 */

asmlinkage long sys_rt_sigreturn(void);

asmlinkage long sys_mmap2(unsigned long addr, unsigned long len,
            unsigned long prot, unsigned long flags,
            unsigned long fd, off_t pgoff);

#endif /* __ASM_ECO32_SYSCALLS_H */
