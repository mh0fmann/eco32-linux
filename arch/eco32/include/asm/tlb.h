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
 * tlb.h -- functions for maipulations tlb entires
 */

#ifndef __ASM_ECO32_TLB_H
#define __ASM_ECO32_TLB_H

#include <asm/eco32.h>


#define NUM_TLB_ENTRIES		32
#define INVALID_PAGE		ECO32_KERNEL_DIRECT_MAPPED_RAM_START


void set_tlb_handler(void);

/*
 * ECO32 doesn't need any special per-pte or
 * per-vma handling..
 */
#define tlb_start_vma(tlb, vma) do { } while (0)
#define tlb_end_vma(tlb, vma) do { } while (0)


#define flush_tlb_entry(idx)	set_tlb((idx), INVALID_PAGE, 0)

static inline void set_tlb(unsigned int idx, unsigned int virt, unsigned int phys)
{
	__eco32_write_tlbidx(idx);
	__eco32_write_tlbhi(virt);
	__eco32_write_tlblo(phys);
	__eco32_write_tlbi();
}

static inline void set_tlbr(unsigned int virt, unsigned int phys)
{
	__eco32_write_tlbhi(virt);
	__eco32_write_tlblo(phys);
	__eco32_write_tlbr();
}

static inline unsigned long read_tlb_hi(unsigned int idx)
{
	__eco32_write_tlbidx(idx);
	__eco32_read_tlbi();
	return __eco32_read_tlbhi();
}

static inline unsigned long read_tlb_lo(unsigned int idx)
{
	__eco32_write_tlbidx(idx);
	__eco32_read_tlbi();
	return __eco32_read_tlblo();
}

static inline int find_tlb_addr(unsigned long addr)
{
	__eco32_write_tlbhi(addr);
	__eco32_search_tlb();
	return __eco32_read_tlbidx();
}


#define tlb_flush(tlb) flush_tlb_mm((tlb)->mm)
#include <linux/pagemap.h>
#include <asm-generic/tlb.h>

#endif /* __ASM_ECO32_TLB_H */
