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
#include <linux/unistd.h>
#include <asm/syscalls.h>


#undef __SYSCALL
#define __SYSCALL(nr, call) [nr] = (sys_call_t)(call),


/* syscall table */
sys_call_t syscall_table[__NR_syscalls] = {
    [0 ... __NR_syscalls-1] = (sys_call_t)sys_ni_syscall,
#include <asm/unistd.h>
};


