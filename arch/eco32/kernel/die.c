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

#include <linux/printk.h>
#include <linux/signal.h>
#include <linux/sched.h>

#include <asm/bug.h>

void die(char* msg, struct pt_regs* regs, long err)
{
    console_verbose();
    printk("\n%s#: %04lx\n", msg, err & 0xffff);
    show_regs(regs);
    do_exit(SIGSEGV);
}
