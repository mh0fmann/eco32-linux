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


#include <linux/mm_types.h>
#include <asm/tlbflush.h>
#include <asm/tlb.h>
#include <asm/irq.h>
#include <asm/eco32.h>
#include <asm/thread_info.h>


extern void do_page_fault(struct pt_regs* regs, int write);


/* 
 * All tlb miss arrive here if umiss could not handle them.
 * We just neeed to differ between read and write miss
 */
static void do_tlb_miss(int irqnr, struct pt_regs* regs)
{
    unsigned long write = irqnr == XCPT_TLB_WRITE ? 1 : 0;
    do_page_fault(regs, write);
}


void __init set_tlb_handler(void)
{
    set_ISR(XCPT_TLB_MISS, do_tlb_miss);
    set_ISR(XCPT_TLB_WRITE, do_tlb_miss);
    set_ISR(XCPT_TLB_INVAL, do_tlb_miss);
}


void flush_tlb_all(void)
{
    int i;
    /* set invalid page for all entries */
    for (i = 0; i < NUM_TLB_ENTRIES; i++) {
        set_tlb(i, INVALID_PAGE, 0);
    }
}


void flush_tlb_page(struct vm_area_struct* vma, unsigned long addr)
{
    int idx = find_tlb_addr(addr);

    if (idx >= 0) {
        flush_tlb_entry(idx);
    }
}


void flush_tlb_range(struct vm_area_struct* vma,
                     unsigned long start,
                     unsigned long end)
{
    int i;
    for (i = 0; i < NUM_TLB_ENTRIES; i++) {
        unsigned long hi = read_tlb_hi(i);
        if (hi <= start && hi <= end) {
            flush_tlb_entry(i);
        }
    }
}
