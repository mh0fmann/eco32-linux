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

#ifndef _ASM_THREAD_INFO_H
#define _ASM_THREAD_INFO_H

#ifdef __KERNEL__

#ifndef __ASSEMBLY__

#include <asm/types.h>
#include <asm/processor.h>
#endif

/* THREAD_SIZE is the size of the task_struct/kernel_stack combo.
 * normally, the stack is found by doing something like p + THREAD_SIZE
 * in ECO32, a page is 4096 bytes, which seems like a sane size
 */

#define THREAD_SIZE_ORDER 0
#define THREAD_SIZE       (PAGE_SIZE << THREAD_SIZE_ORDER)


#ifndef __ASSEMBLY__

typedef unsigned long mm_segment_t;

struct thread_info {
    struct task_struct* task;          /* main task structure */
    unsigned long       flags;         /* low level flags */
    __u32               cpu;           /* current CPU */
    __s32               preempt_count; /* 0: preemptable, <0: BUG */

    mm_segment_t        addr_limit;    /* thread address space:
                                          0-0x7FFFFFFF user-thead
                                          0-0xFFFFFFFF kernel-thread */
    /* saved context is on kernel stack */
    unsigned long       ksp; /* where to find it */
};


/*
 * macros/functions for gaining access to the thread information structure
 *
 * preempt_count needs to be 1 initially, until the scheduler is functional.
 */
#define INIT_THREAD_INFO(tsk)   \
{                               \
    .task          = &tsk,      \
    .flags         = 0,         \
    .cpu           = 0,         \
    .preempt_count = 1,         \
    .addr_limit    = KERNEL_DS, \
    .ksp           = 0,         \
}


/* how to get the thread information struct from C */
register struct thread_info* current_thread_info_reg asm("$27");
#define current_thread_info()   (current_thread_info_reg)


#endif /* !__ASSEMBLY__ */

#define TIF_SYSCALL_TRACE       0 /* syscall trace active */
#define TIF_NOTIFY_RESUME       1 /* resumption notification requested */
#define TIF_SIGPENDING          2 /* signal pending */
#define TIF_NEED_RESCHED        3 /* rescheduling necessary */
#define TIF_SINGLESTEP          4 /* restore singlestep on return to user mode */
#define TIF_SYSCALL_TRACEPOINT  8 /* for ftrace syscall instrumentation */
#define TIF_RESTORE_SIGMASK     9
#define TIF_POLLING_NRFLAG      16 /* true if poll_idle() is polling TIF_NEED_RESCHED */
#define TIF_MEMDIE              17

#define _TIF_SYSCALL_TRACE      (1<<TIF_SYSCALL_TRACE)
#define _TIF_NOTIFY_RESUME      (1<<TIF_NOTIFY_RESUME)
#define _TIF_SIGPENDING         (1<<TIF_SIGPENDING)
#define _TIF_NEED_RESCHED       (1<<TIF_NEED_RESCHED)
#define _TIF_SINGLESTEP         (1<<TIF_SINGLESTEP)
#define _TIF_POLLING_NRFLAG     (1<<TIF_POLLING_NRFLAG)

/* Work to do when returning from interrupt/exception */
#define _TIF_WORK_MASK          (0xff & ~(_TIF_SYSCALL_TRACE|_TIF_SINGLESTEP|_TIF_NOTIFY_RESUME))

#endif /* __KERNEL__ */

#endif /* _ASM_THREAD_INFO_H */
