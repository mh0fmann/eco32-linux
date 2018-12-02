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

#ifndef _UAPI__ASM_ECO32_PTRACE_H
#define _UAPI__ASM_ECO32_PTRACE_H

#ifndef __ASSEMBLY__

/*
 * This is the layout of the regset returned by the GETREGSET ptrace call.
 */

struct user_regs_struct {
    unsigned long gpr[32];
    unsigned long pc;
    unsigned long sr;
};

#endif

#endif /* _UAPI__ASM_ECO32_PTRACE_H */
