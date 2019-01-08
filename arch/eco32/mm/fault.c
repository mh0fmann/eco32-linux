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
#include <linux/sched/signal.h>

#include <asm/signal.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/eco32.h>
#include <asm/mmu_context.h>


void do_page_fault(struct pt_regs* regs, int write)
{
	struct task_struct* tsk = current;
	struct mm_struct* mm = tsk->mm;
	struct vm_area_struct* vma = NULL;
	int fault;
    int si_code;
	unsigned int flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;
	unsigned int addr;

	addr = __eco32_read_tlbbad();
	pr_debug("fixing address 0x%08x\n", addr);
	pr_debug("current_pgd: %p\n", current_pgd);

	if (addr >= VMALLOC_START && addr < VMALLOC_END)
		goto vmalloc_fault;

	if (user_mode(regs))
		flags |= FAULT_FLAG_USER;

	si_code = SEGV_MAPERR;

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
		force_sig_fault(SIGSEGV, si_code, (void*)addr, current);
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

	force_sig_fault(SIGBUS, si_code, (void __user*)addr, current);

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
