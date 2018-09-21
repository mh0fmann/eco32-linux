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

#ifndef __ASM_ECO32_MMU_CONTEXT_H
#define __ASM_ECO32_MMU_CONTEXT_H

#include <asm-generic/mm_hooks.h>
#include <asm/tlb.h>

#define NO_CONTEXT -1

/*
 * pointer to the pgd of the current process
 */
extern volatile pgd_t* current_pgd;


static inline int init_new_context(struct task_struct* tsk, struct mm_struct* mm)
{
    mm->context = NO_CONTEXT;
    return 0;
}

#define destroy_context(mm)	flush_tlb_mm((mm))

static inline void switch_mm(struct mm_struct* prev, struct mm_struct* next, struct task_struct* next_tsk)
{
    current_pgd = next->pgd;

    if (prev != next) {
        flush_tlb_mm(prev);
    }
}

#define deactivate_mm(tsk, mm)      do { } while (0)
#define activate_mm(prev, next)     switch_mm((prev), (next), NULL)
#define enter_lazy_tlb(mm, tsk)     do { } while (0)

#endif
