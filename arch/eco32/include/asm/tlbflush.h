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

#ifndef __ASM_ECO32_TLBFLUSH_H
#define __ASM_ECO32_TLBFLUSH_H

#include <linux/mm.h>
#include <asm/processor.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/current.h>
#include <linux/sched.h>

/*
 *  - flush_tlb() flushes the current mm struct TLBs
 *  - flush_tlb_all() flushes all processes TLBs
 *  - flush_tlb_mm(mm) flushes the specified mm context TLB's
 *  - flush_tlb_page(vma, vmaddr) flushes one page
 *  - flush_tlb_range(mm, start, end) flushes a range of pages
 */

void flush_tlb_all(void);
void flush_tlb_page(struct vm_area_struct* vma, unsigned long addr);
void flush_tlb_range(struct vm_area_struct* vma,
                     unsigned long start,
                     unsigned long end);

static inline void flush_tlb_mm(struct mm_struct* mm)
{
    /* needs improvement */
    flush_tlb_all();
}

static inline void flush_tlb(void)
{
    flush_tlb_mm(current->mm);
}

static inline void flush_tlb_kernel_range(unsigned long start,
        unsigned long end)
{
    flush_tlb_range(NULL, start, end);
}

#endif /* __ASM_ECO32_TLBFLUSH_H */
