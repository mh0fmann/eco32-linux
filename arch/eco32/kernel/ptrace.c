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


#include <linux/kernel.h>
#include <linux/ptrace.h>
#include <linux/regset.h>
#include <linux/audit.h>
#include <linux/tracehook.h>
#include <linux/errno.h>
#include <linux/elf.h>

#include <asm/elf.h>


long arch_ptrace(struct task_struct* child, long request,
                 unsigned long addr, unsigned long data)
{
    return ptrace_request(child, request, addr, data);
}


void ptrace_disable(struct task_struct* child)
{
    clear_tsk_thread_flag(child, TIF_SYSCALL_TRACE);
}


static int eco32_get(struct task_struct *target,
                     const struct user_regset *regset,
                     unsigned int pos, unsigned int count,
                     void *kbuf, void __user *ubuf)
{
    struct pt_regs *regs;

    regs = task_pt_regs(target);
    return user_regset_copyout(&pos, &count, &kbuf, &ubuf, regs, 0, -1);
}


static int eco32_set(struct task_struct *target,
                     const struct user_regset *regset,
                     unsigned int pos, unsigned int count,
                     const void *kbuf, const void __user *ubuf)
{
    int ret;
    struct pt_regs *regs;

    regs = task_pt_regs(target);
    ret = user_regset_copyin(&pos, &count, &kbuf, &ubuf, regs, 0, -1);
    return ret;
}


enum eco32_regset{
    REGSET_X
};


static const struct user_regset eco32_user_regset[] = {
    [REGSET_X] = {
        .core_note_type = NT_PRSTATUS,
        .n = ELF_NGREG,
        .size = sizeof(elf_greg_t),
        .align = sizeof(elf_greg_t),
        .get = &eco32_get,
        .set = &eco32_set,
    },
};


static const struct user_regset_view eco32_user_native_view = {
    .name = "eco32",
    .e_machine = EM_ECO32,
    .regsets = eco32_user_regset,
    .n = ARRAY_SIZE(eco32_user_regset),
};


const struct user_regset_view* task_user_regset_view(struct task_struct* task)
{
    return &eco32_user_native_view;
}


/*
 * Notification of system call entry/exit
 * - triggered by current->work.syscall_trace
 */
long syscall_trace_enter(struct pt_regs* regs)
{
    long ret = 0;

    if (test_thread_flag(TIF_SYSCALL_TRACE) &&
        tracehook_report_syscall_entry(regs))
        /*
         * Tracing decided this syscall should not happen.
         * We'll return a bogus call number to get an ENOSYS
         * error, but leave the original number in <something>.
         */
        ret = -1L;

    audit_syscall_entry(regs->gpr[11], regs->gpr[3], regs->gpr[4],
                        regs->gpr[5], regs->gpr[6]);

    return ret ? : regs->gpr[11];
}


void syscall_trace_leave(struct pt_regs* regs)
{
    int step;

    audit_syscall_exit(regs);

    step = test_thread_flag(TIF_SINGLESTEP);

    if (step || test_thread_flag(TIF_SYSCALL_TRACE))
        tracehook_report_syscall_exit(regs, step);
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


void show_stack(struct task_struct* tsk, unsigned long* sp)
{
    /* not implemented */
}


