/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * highmem.c -- virtual kernel memory mappings for high memory
 */


#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/preempt.h>
#include <linux/uaccess.h>
#include <linux/highmem.h>
#include <asm/highmem.h>
#include <asm/fixmap.h>
#include <asm/tlbflush.h>


unsigned long highstart_pfn, highend_pfn;

static pte_t* kmap_pte;


void __init kmap_init(void)
{
	unsigned long phys_kmap_pte;
	pgd_t* dir;
	pte_t* pte;
	
	kmap_pte = (pte_t*)__fix_to_virt(FIX_PKMAP);
	
	phys_kmap_pte = memblock_alloc(PAGE_SIZE, PAGE_SIZE);
	memset(__va(phys_kmap_pte), 0x0, PAGE_SIZE);

	dir = pgd_offset_k(__fix_to_virt(FIX_PKMAP));

	pte = (pte_t*)pgd_val(*dir);
	pte += __pte_offset(FIX_PKMAP);

	pte->pte = phys_kmap_pte | _PAGE_WRITE;
}


void *kmap(struct page *page)
{
	void *addr;

	might_sleep();
	if (!PageHighMem(page))
		return page_address(page);
	addr = kmap_high(page);
	flush_tlb_all();

	return addr;
}

void kunmap(struct page *page)
{
	BUG_ON(in_interrupt());
	if (!PageHighMem(page))
		return;
	kunmap_high(page);;
}


void *kmap_atomic(struct page *page)
{
	unsigned long vaddr;
	int idx, type;

	preempt_disable();
	pagefault_disable();
	if (!PageHighMem(page))
		return page_address(page);

	type = kmap_atomic_idx_push();
	idx = type + KM_TYPE_NR*smp_processor_id();
	vaddr = __fix_to_virt(FIX_PKMAP + idx);

	set_pte(kmap_pte-idx, mk_pte(page, PAGE_KERNEL));
	flush_tlb_all();

	return (void*) vaddr;
}


void __kunmap_atomic(void *kvaddr)
{
	unsigned long vaddr = (unsigned long) kvaddr & PAGE_MASK;
	int type __maybe_unused;

	if (vaddr < PKMAP_BASE || vaddr > PKMAP_BASE + LAST_PKMAP * PAGE_SIZE) {
		pagefault_enable();
		preempt_enable();
		return;
	}

	type = kmap_atomic_idx();

	kmap_atomic_idx_pop();
	pagefault_enable();
	preempt_enable();
}
