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
 * sigcontext.h -- signal context definitions for ECO32
 */

#ifndef __ASM_ECO32_SIGCONTEXT_H
#define __ASM_ECO32_SIGCONTEXT_H

#include <asm/ptrace.h>

/*
 * This struct is saved by setup_frame in signal.c, to keep
 * the current context while a signal handler is executed.
 * It's restored by sys_sigreturn.
 */

struct sigcontext {
	struct user_regs_struct regs;  /* needs to be first */
	unsigned long oldmask;
};

#endif /* __ASM_ECO32_SIGCONTEXT_H */
