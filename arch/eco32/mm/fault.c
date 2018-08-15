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
 * fault.c -- handle memory management faults
 */

#include <asm/ptrace.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/extable.h>
#include <linux/sched/signal.h>

#include <asm/siginfo.h>
#include <asm/signal.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/eco32.h>
#include <asm/irq.h>

#include <asm/mmu_context.h>

/*
 * this is the pointer to the pgd of the current running process!
 * if a context switch occours this pointer needs to be updated
 * other wise the tlb miss function can not update the tlb entries
 */
volatile pgd_t* current_pgd;
extern void die(char* msg, struct pt_regs* regs, long err);



int fixup_exception(struct pt_regs* regs)
{
	const struct exception_table_entry* entry;
	entry = search_exception_tables(regs->xa);

	if (entry) {
		pr_debug("fixup 0x%08lx found for insn at 0x%08lx\n",
		         entry->fixup, regs->xa);
		regs->xa = entry->fixup;
		return 1;
	}

	pr_debug("no fixup found for insn at 0x%08lx\n",
	         regs->xa);
	return 0;
}


asmlinkage void do_page_fault(struct pt_regs* regs, int write)
{
	struct task_struct* tsk = current;
	struct mm_struct* mm = tsk->mm;
	struct vm_area_struct* vma = NULL;
	siginfo_t info;
	int fault;
	unsigned int flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;
	unsigned int addr;

	addr = __eco32_read_tlbbad();
	pr_debug("fixing address 0x%08x\n", addr);
	pr_debug("current_pgd: %p\n", current_pgd);

	if (addr >= VMALLOC_START && addr < VMALLOC_END)
		goto vmalloc_fault;

	if (user_mode(regs))
		flags |= FAULT_FLAG_USER;

	info.si_code = SEGV_MAPERR;

	if (in_interrupt() || !mm) {
		goto no_context;
	}

retry:
	down_read(&mm->mmap_sem);
	vma = find_vma(mm, addr);

	if (!vma)
		goto bad_area;

	if (vma->vm_start <= addr)
		goto good_area;

	if (!(vma->vm_flags & VM_GROWSDOWN))
		goto bad_area;

	if (expand_stack(vma, addr))
		goto bad_area;

good_area:

	if (write) {
		if (!(vma->vm_flags & VM_WRITE))
			goto bad_area;

		flags |= FAULT_FLAG_WRITE;
	} else {
		if (!(vma->vm_flags & (VM_READ | VM_EXEC)))
			goto bad_area;
	}


	fault = handle_mm_fault(vma, addr, flags);

	if ((fault & VM_FAULT_RETRY) && fatal_signal_pending(current))
		return;

	if (fault & VM_FAULT_ERROR) {
		if (fault & VM_FAULT_OOM)
			goto out_of_memory;
		else if (fault & VM_FAULT_SIGSEGV)
			goto bad_area;
		else if (fault & VM_FAULT_SIGBUS)
			goto do_sigbus;
	}

	if (flags & FAULT_FLAG_ALLOW_RETRY) {
		if (fault & VM_FAULT_MAJOR)
			tsk->maj_flt++;
		else
			tsk->min_flt++;

		if (fault & VM_FAULT_RETRY) {
			flags &= ~FAULT_FLAG_ALLOW_RETRY;
			flags |= FAULT_FLAG_TRIED;
			goto retry;
		}
	}

	up_read(&mm->mmap_sem);
	return;

bad_area:
	up_read(&mm->mmap_sem);

bad_area_nosemaphore:

	/* Usermode Segfault*/
	if (user_mode(regs)) {
		info.si_signo = SIGSEGV;
		info.si_errno = 0;
		info.si_addr = (void*)addr;
		force_sig_info(SIGSEGV, &info, tsk);
		return;
	}

no_context:

	/* fix for valid kernel exception points */
	if (fixup_exception(regs)) {
		return;
	}

	/* Oops handling */
	if ((unsigned long)addr < PAGE_SIZE)
		pr_alert("Unable to handle kernel NULL pointer dereferencing\n");
	else
		pr_alert("Unable to handle kernel access at virtual address 0x%08x\n",
		         addr);

	die("Oops", regs, /*write access*/ 0); /*fix write access*/
	do_exit(SIGKILL);

out_of_memory:
	up_read(&mm->mmap_sem);

	if (!user_mode(regs))
		goto no_context;

	pagefault_out_of_memory();
	return;

do_sigbus:
	up_read(&mm->mmap_sem);

	info.si_signo = SIGBUS;
	info.si_errno = 0;
	info.si_code = BUS_ADRERR;
	info.si_addr = (void*)addr;
	force_sig_info(SIGBUS, &info, tsk);

	if (!user_mode(regs))
		goto no_context;

	return;

vmalloc_fault: {
		int offset = pgd_index(addr);
		pgd_t* pgd, *pgd_k;
		p4d_t* p4d, *p4d_k;
		pud_t* pud, *pud_k;
		pmd_t* pmd, *pmd_k;
		pte_t* pte_k;

		pgd = (pgd_t*)(current_pgd + offset);
		pgd_k = init_mm.pgd + offset;

		p4d = p4d_offset(pgd, addr);
		p4d_k = p4d_offset(pgd_k, addr);

		if (!p4d_present(*p4d_k))
			goto no_context;

		pud = pud_offset(p4d, addr);
		pud_k = pud_offset(p4d_k, addr);

		if (!pud_present(*pud_k))
			goto no_context;

		pmd = pmd_offset(pud, addr);
		pmd_k = pmd_offset(pud_k, addr);

		if (!pmd_present(*pmd_k))
			goto bad_area_nosemaphore;

		set_pmd(pmd, *pmd_k);

		pte_k = pte_offset_kernel(pmd_k, addr);

		if (!pte_present(*pte_k))
			goto no_context;

		return;
	}

}
