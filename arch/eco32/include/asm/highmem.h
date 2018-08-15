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
 * highmem.h -- virtual kernel memory mappings for high memory
 */


#ifndef __ASM_ECO32_HIGHMEM_H
#define __ASM_ECO32_HIGHMEM_H

#ifdef __KERNEL__

#include <linux/init.h>
#include <asm/kmap_types.h>
#include <asm/tlbflush.h>
#include <asm/fixmap.h>
#include <asm/eco32.h>
#include <asm/page.h>


/* declarations for highmem.c */
extern unsigned long highstart_pfn, highend_pfn;

/*
 * Right now we initialize only a single pte table. It can be extended
 * easily, subsequent pte tables have to be allocated in one physical
 * chunk of RAM.
 */

#define kmap_prot		PAGE_KERNEL

#define LAST_PKMAP		1024
#define LAST_PKMAP_MASK (LAST_PKMAP-1)
#define PKMAP_BASE		(ECO32_KERNEL_DIRECT_MAPPED_RAM_START - PAGE_SIZE * PTRS_PER_PTE)
#define PKMAP_ADDR(nr)	(PKMAP_BASE + ((nr) << PAGE_SHIFT))
#define PKMAP_NR(virt)	(((virt) - PKMAP_BASE) >> PAGE_SHIFT)


extern void *kmap_high(struct page *page);
extern void kunmap_high(struct page *page);

void *kmap(struct page *page);
void kunmap(struct page *page);

void *kmap_atomic(struct page *page);
void __kunmap_atomic(void *kvaddr);

void __init kmap_init(void);

#define flush_cache_kmaps()	flush_tlb_all()


#endif /* __KERNEL__ */

#endif /* __ASM_ECO32_HIGHMEM_H */
