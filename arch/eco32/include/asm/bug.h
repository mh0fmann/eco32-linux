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

#ifndef __ASM_ECO32_BUG_H
#define __ASM_ECO32_BUG_H


#ifndef __ASSAMBLY__

#include <asm/ptrace.h>

void die(char* msg, struct pt_regs* regs, long err);
void show_regs(struct pt_regs* regs);

#endif


#include <asm-generic/bug.h>

#endif
