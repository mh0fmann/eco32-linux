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


#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/tracehook.h>
#include <linux/unistd.h>
#include <linux/err.h>
#include <linux/uaccess.h>

#include <asm/ucontext.h>
#include <asm/cacheflush.h>


#define INSN_ORI_R2_R0      0x4C020000
#define INSN_TRAP           0xB8000000


struct rt_sigframe {
    struct siginfo info;        /* signal descriptor       */
    struct ucontext uc;     /* usermode context        */
    unsigned long retcode[2];   /* trampoline code:        */
    /*   ori   $2,$0,NR_SIGRET */
    /*   trap                  */
};


/* ---------------------------------------------------------- */


/*
 * Restore original process context.
 */
static int restore_sigcontext(struct sigcontext __user* sc,
                              struct pt_regs* regs)
{
    int err = 0;

    current->restart_block.fn = do_no_restart_syscall;
    err |= __copy_from_user(regs, sc->regs.gpr,
                            32 * sizeof(unsigned long));
    err |= __copy_from_user(&regs->xa, &sc->regs.pc,
                            sizeof(unsigned long));
    err |= __copy_from_user(&regs->psw, &sc->regs.sr,
                            sizeof(unsigned long));
    return err;
}


/*
 * This function implements the "sigreturn" syscall.
 */
asmlinkage long sys_rt_sigreturn(void)
{

    struct pt_regs* regs = current_pt_regs();
    struct rt_sigframe __user* frame;

    pr_debug("do_sigreturn called, regs @ 0x%08lx\n",
             (unsigned long) regs);

    /*
     * Since we stacked the signal on a 32-bit boundary,
     * sp should be word aligned here. If it's not, then
     * the user is trying to mess with us.
     */
    if (regs->sp & 3) {
        goto badframe;
    }

    frame = (struct rt_sigframe __user*) regs->sp;

    /*
     * Restore sigcontext and check various error conditions.
     */
    if (!access_ok(frame, sizeof(struct rt_sigframe))) {
        goto badframe;
    }

    if (restore_sigcontext(&frame->uc.uc_mcontext, regs)) {
        goto badframe;
    }

    if (restore_altstack(&frame->uc.uc_stack)) {
        goto badframe;
    }

    return regs->rv;

badframe:
    force_sig(SIGSEGV, current);
    return 0;
}


/* ---------------------------------------------------------- */


/*
 * Align a given stack pointer to word boundary and
 * convert it to the correct data type for sigframes.
 */
static inline void __user* align_sigframe(unsigned long sp)
{
    return (void __user*) (sp & ~3UL);
}


/*
 * Allocate a frame for signal handling on the
 * user's stack, and check write permission.
 */
static inline void __user* get_sigframe(struct ksignal* ksig,
                                        struct pt_regs* regs,
                                        size_t framesize)
{
    unsigned long sp;
    void __user* frame;

    sp = sigsp(regs->sp, ksig);
    frame = align_sigframe(sp - framesize);

    if (!access_ok(frame, framesize)) {
        frame = NULL;
    }

    return frame;
}


/*
 * Save original process context.
 */
static int setup_sigcontext(struct sigcontext __user* sc,
                            struct pt_regs* regs)
{
    int err = 0;

    err |= __copy_to_user(sc->regs.gpr, regs,
                          32 * sizeof(unsigned long));
    err |= __copy_to_user(&sc->regs.pc, &regs->xa,
                          sizeof(unsigned long));
    err |= __copy_to_user(&sc->regs.sr, &regs->psw,
                          sizeof(unsigned long));

    return err;
}


/*
 * Set up a frame for executing the signal handler.
 */
static int setup_rt_frame(struct ksignal* ksig,
                          sigset_t* set,
                          struct pt_regs* regs)
{
    struct rt_sigframe __user* frame;
    int err = 0;
    unsigned long __user* retcode;

    pr_debug("setting up signal frame, regs @ 0x%08lx\n",
             (unsigned long) regs);

    frame = get_sigframe(ksig, regs, sizeof(struct rt_sigframe));

    if (frame == NULL) {
        /* could not allocate a frame with write permission */
        return 1;
    }

    if (ksig->ka.sa.sa_flags & SA_SIGINFO) {
        err |= copy_siginfo_to_user(&frame->info, &ksig->info);
    }

    /* create user context */
    err |= __put_user(0, &frame->uc.uc_flags);
    err |= __put_user(0, &frame->uc.uc_link);
    err |= __save_altstack(&frame->uc.uc_stack, regs->sp);
    err |= setup_sigcontext(&frame->uc.uc_mcontext, regs);
    err |= __copy_to_user(&frame->uc.uc_sigmask, set, sizeof(sigset_t));
    /* return from userspace */
    retcode = (unsigned long __user*) &frame->retcode;
    err |= __put_user(INSN_ORI_R2_R0 | __NR_rt_sigreturn, retcode + 0);
    err |= __put_user(INSN_TRAP,                          retcode + 1);
    /* setup registers for signal handler */
    regs->xa = (unsigned long) ksig->ka.sa.sa_handler;
    regs->ra = (unsigned long) retcode;
    regs->sp = (unsigned long) frame;
    /* setup args */
    regs->r4 = ksig->sig;
    regs->r5 = (unsigned long) &frame->info;
    regs->r6 = (unsigned long) &frame->uc;

    flush_cache_all();

    return err;
}


/*
 * Prepare usermode signal handling.
 */
static inline void handle_signal(struct ksignal* ksig,
                                 struct pt_regs* regs)
{
    sigset_t* oldset;
    int err;

    pr_debug("handle_signal called, regs @ 0x%08lx\n",
             (unsigned long) regs);

    oldset = sigmask_to_save();
    err = setup_rt_frame(ksig, oldset, regs);
    signal_setup_done(err, ksig, 0);
}


/*
 * Act on a pending signal.
 */
void do_signal(struct pt_regs* regs)
{
    struct ksignal ksig;
    int syscall_num;
    int syscall_err;

    pr_debug("do_signal called, regs @ 0x%08lx\n",
             (unsigned long) regs);

    /* in simple cases, get_signal() does not even return */
    if (get_signal(&ksig)) {
        /* we have a signal to deliver */
        handle_signal(&ksig, regs);
        return;
    }


    /* did we come from a syscall? */
    syscall_num = regs->r2_orig;

    if (syscall_num >= 0) {
        syscall_err = IS_ERR_VALUE(regs->r2) ? regs->r2 : 0;

        switch (syscall_err) {
            case -ERESTARTNOHAND:
            case -ERESTARTSYS:
            case -ERESTARTNOINTR:
                regs->r2 = regs->r2_orig;
                regs->xa -= 4;
                break;

            case -ERESTART_RESTARTBLOCK:
                regs->r2 = __NR_restart_syscall;
                regs->xa -= 4;
                break;
        }
    }

    /* put saved sigmask back */
    restore_saved_sigmask();
}


/* ---------------------------------------------------------- */


/*
 * There is work pending for this thread.
 * Three cases are handled:
 *   - rescheduling
 *   - signal delivery
 *   - notify resume
 */
void do_work_pending(struct pt_regs* regs,
                     unsigned int thread_flags)
{
    if (thread_flags & _TIF_NEED_RESCHED) {
        schedule();
        return;
    }

    local_irq_enable();

    if (thread_flags & _TIF_SIGPENDING) {
        do_signal(regs);
        return;
    }

    if (thread_flags & _TIF_NOTIFY_RESUME) {
        clear_thread_flag(TIF_NOTIFY_RESUME);
        tracehook_notify_resume(regs);
        return;
    }

    /* should never be reached */
    panic("%s: unknown thread_flags 0x%08x",
          __func__, thread_flags);
}
