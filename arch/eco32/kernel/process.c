/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Hellwig Geisse, Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * process.c -- thread handling
 */


#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/export.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/personality.h>
#include <linux/sys.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/kallsyms.h>
#include <linux/random.h>
#include <linux/prctl.h>


/*
 * Pointer to current thread info structure.
 * Used at user -> kernel transitions.
 */
struct thread_info* current_ti = &init_thread_info;


/*
 * If a process does an exec syscall, machine state
 * like FPU and debug registers need to be reset.
 * This is a hook function for that purpose.
 */
void flush_thread(void)
{
	/* nothing to do here */
}


extern __visible asmlinkage void ret_from_fork(void);


/*
 * Copy arch-specific thread state.
 *
 * At the top of a newly initialized kernel stack are two
 * stacked pt_reg structures. The first (topmost) one is
 * the userspace context of the thread. The second one is
 * the kernelspace context of the thread.
 *
 * A kernel thread will usually not be returning to userspace,
 * so the topmost pt_reg structure can be left uninitialized.
 * It does need to exist however, because a kernel thread may
 * mutate to a userspace thread later by doing a kernel_execve
 * (see below).
 *
 * The second pt_reg structure needs to be initialized to
 * 'return' to ret_from_fork. A kernel thread needs to set
 * $16 to the address of the 'kernel thread function' (which
 * will be called from within ret_from fork), and $17 to its
 * single argument. A userspace thread needs to set $16 to
 * NULL, in which case ret_from_fork will just continue to
 * return to userspace.
 *
 * A kernel thread function may return: this is what happens
 * when kernel_execve is called. In that case, the userspace
 * pt_regs must have been initialized (which kernel_execve
 * takes care of, see start_thread below); ret_from_fork will
 * then continue its execution causing the 'kernel thread' to
 * return to userspace as a userspace thread.
 */
int copy_thread(unsigned long clone_flags, unsigned long usp,
                unsigned long arg, struct task_struct* p)
{
	struct thread_info* ti;
	unsigned long top_of_kernel_stack;
	unsigned long sp;
	struct pt_regs* uregs;
	struct pt_regs* kregs;

	p->set_child_tid = NULL;
	p->clear_child_tid = NULL;

	/* locate thread info and top of kernel stack */
	ti = task_thread_info(p);
	top_of_kernel_stack = (unsigned long) ti + THREAD_SIZE;
	sp = top_of_kernel_stack;

	/* locate userspace context on stack */
	sp -= sizeof(struct pt_regs);
	uregs = (struct pt_regs*) sp;

	/* locate kernel context on stack */
	sp -= sizeof(struct pt_regs);
	kregs = (struct pt_regs*) sp;

	if (unlikely(p->flags & PF_KTHREAD)) {
		/* create kernel thread */
		/* may later mutate to userspace thread */
		pr_debug("create kernel thread, info @ 0x%08lx\n",
		         (unsigned long) ti);
		memset(kregs, 0, sizeof(struct pt_regs));
		kregs->gpr[16] = usp;	/* kernel thread function */
		kregs->gpr[17] = arg;	/* kernel thread argument */
	} else {
		/* create userspace thread */
		/* (in response to fork() syscall) */
		pr_debug("create userspace thread, info @ 0x%08lx\n",
		         (unsigned long) ti);
		*uregs = *current_pt_regs();

		if (usp) {
			uregs->sp = usp;
		}

		uregs->gpr[2] = 0;	/* result from fork() */
		kregs->gpr[16] = 0;	/* this is a userspace thread */
	}

	/* every newly created thread will resume execution here */
	kregs->gpr[31] = (unsigned long) ret_from_fork;

	/* ti->ksp is used to locate the stored kernel context */
	ti->ksp = (unsigned long) kregs;

	return 0;
}


/*
 * Set up a thread for executing a new program.
 */
void start_thread(struct pt_regs* regs, unsigned long pc, unsigned long sp)
{
	unsigned long psw = 0x0A40FFFF;

	pr_debug("start_thread called, regs @ 0x%08lux pc=0x%08lux sp=0x%08lux\n",
	         (unsigned long) regs, pc, sp);
	memset(regs, 0, sizeof(struct pt_regs));
	regs->psw = psw;
	regs->xa = pc;
	regs->sp = sp;
}


/*
 * Return saved PC of a blocked thread.
 */
unsigned long thread_saved_pc(struct task_struct* tsk)
{
	return (unsigned long) user_regs(tsk->stack)->xa;
}


/*
 * ???
 */
void release_thread(struct task_struct* tsk)
{
	/* nothing to do here */
}


extern struct thread_info* _switch(struct thread_info* old_ti,
                                   struct thread_info* new_ti);


/*
 * Switch threads.
 */
struct task_struct* __switch_to(struct task_struct* old,
                                struct task_struct* new)
{
	struct task_struct* last;
	struct thread_info* new_ti, *old_ti;
	unsigned long flags;

	local_irq_save(flags);

	new_ti = new->stack;
	old_ti = old->stack;

	/*
	 * Current_set is an array of saved current pointers
	 * (one for each cpu). We need them at user -> kernel
	 * transitions, while we save them here. For the time
	 * being, we only have a single cpu and thus only
	 * a single current_ti.
	 */

	current_ti = new_ti;
	last = (_switch(old_ti, new_ti))->task;

	local_irq_restore(flags);

	return last;
}


/*
 * Show contents of registers.
 */
void show_regs(struct pt_regs* regs)
{
	char line[80];
	char* p;
	int i, j;
	int rn;
	unsigned long psw;

	for (i = 0; i < 8; i++) {
		p = line;

		for (j = 0; j < 4; j++) {
			rn = 8 * j + i;
			p += sprintf(p, "$%-2d  %08lX     ",
			             rn, regs->gpr[rn]);
		}

		pr_info("%s\n", line);
	}

	psw = regs->psw;
	pr_info("     xxxx  V  UPO  IPO  IACK   MASK\n");
	p = line;
	p += sprintf(p, "PSW  ");

	for (i = 31; i >= 0; i--) {
		if (i == 27 || i == 26 || i == 23 || i == 20 || i == 15) {
			p += sprintf(p, "  ");
		}

		p += sprintf(p, "%c", psw & (1 << i) ? '1' : '0');
	}

	pr_info("%s\n", line);
}


/*
 * ???
 */
unsigned long get_wchan(struct task_struct* p)
{
	/* TODO */
	return 0;
}
