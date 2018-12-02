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

#ifndef __ASM_ECO32_PROCESSOR_H
#define __ASM_ECO32_PROCESSOR_H

#include <asm/page.h>
#include <asm/ptrace.h>

#define STACK_TOP       TASK_SIZE
#define STACK_TOP_MAX   STACK_TOP

/*
 * Default implementation of macro that returns current
 * instruction pointer ("program counter").
 */

#define current_text_addr()     ({ __label__ _l; _l: &&_l; })

/*
 * User space process size. This is hardcoded into a few places,
 * so don't change it unless you know what you are doing.
 */

#define TASK_SIZE       (0x80000000UL)

/*
 * This decides where the kernel will search for a free chunk
 * of vm space during mmap's.
 */

#define TASK_UNMAPPED_BASE      (TASK_SIZE / 8 * 3)

#ifndef __ASSEMBLY__

struct task_struct;

struct thread_struct {
};

/*
 * At user->kernel entry, the pt_regs struct is stacked on the top of the
 * kernel-stack.  This macro allows us to find those regs for a task.
 * Notice that subsequent pt_regs stackings, like recursive interrupts
 * occurring while we're in the kernel, won't affect this - only the first
 * user->kernel transition registers are reached by this (i.e. not regs
 * for running signal handler).
 */

#define user_regs(thread_info) \
    (((struct pt_regs *)((unsigned long)(thread_info) + THREAD_SIZE)) - 1)

/*
 * Dito but for the currently running task.
 */

#define task_pt_regs(task)  user_regs(task_thread_info(task))

#define INIT_SP             (sizeof(init_stack) + (unsigned long) &init_stack)

#define INIT_THREAD         { }

#define KSTK_EIP(tsk)       (task_pt_regs(tsk)->xa)
#define KSTK_ESP(tsk)       (task_pt_regs(tsk)->sp)

void start_thread(struct pt_regs* regs, unsigned long nip, unsigned long sp);
void release_thread(struct task_struct* tsk);
unsigned long get_wchan(struct task_struct* p);


/*
 * If a process does an exec syscall, machine state
 * like FPU and debug registers need to be reset.
 * This is a hook function for that purpose.
 */
extern inline void flush_thread(void)
{
    /* nothing to do here */
}

/*
 * Free current thread data structures etc.
 */
extern inline void exit_thread(struct task_struct* tsk)
{
    /* nothing to do here */
}

/*
 * Free all ressources held by a thread
 */
extern inline void release_thread(struct task_struct* tsk)
{
    /* nothing to do here */
}

/*
 * Return saved PC of a blocked thread. For now, this is the "user" PC
 */
extern unsigned long thread_saved_pc(struct task_struct* t);

#define cpu_relax()             barrier()
#define cpu_relax_lowlatency()  cpu_relax()

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ECO32_PROCESSOR_H */
