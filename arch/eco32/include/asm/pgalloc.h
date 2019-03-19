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

#ifndef __ASM_ECO32_PGALLOC_H
#define __ASM_ECO32_PGALLOC_H

#include <asm/page.h>
#include <asm/pgtable.h>
#include <linux/threads.h>
#include <linux/mm.h>
#include <linux/memblock.h>

extern int mem_init_done;

#define pmd_populate_kernel(mm, pmd, pte)   set_pmd(pmd, __pmd((unsigned long)pte))

static inline void pmd_populate(struct mm_struct* mm, pmd_t* pmd,
                                struct page* pte)
{
    set_pmd(pmd, __pmd(((unsigned long) __va((unsigned long)page_to_pfn(pte) <<
                       (unsigned long) PAGE_SHIFT))));
}

/*
 * Allocate and free page tables.
 */
static inline pgd_t* pgd_alloc(struct mm_struct* mm)
{
    pgd_t* ret = (pgd_t*)__get_free_page(GFP_KERNEL);

    if (ret) {
        memset(ret, 0, USER_PTRS_PER_PGD * sizeof(pgd_t));
        memcpy(ret + USER_PTRS_PER_PGD,
               swapper_pg_dir + USER_PTRS_PER_PGD,
               KRNL_PTRS_PER_PGD * sizeof(pgd_t));

    }

    return ret;
}

static inline void pgd_free(struct mm_struct* mm, pgd_t* pgd)
{
    free_page((unsigned long)pgd);
}

static inline pte_t* pte_alloc_one_kernel(struct mm_struct* mm)
{
    return (pte_t*)__get_free_page(GFP_KERNEL | ___GFP_RETRY_MAYFAIL | __GFP_ZERO);
}

static inline struct page* pte_alloc_one(struct mm_struct* mm)
{
    struct page* pte;
    pte = alloc_pages(GFP_KERNEL, 0);

    if (!pte) {
        return NULL;
    }

    clear_page(page_address(pte));

    if (!pgtable_page_ctor(pte)) {
        __free_page(pte);
        return NULL;
    }

    return pte;
}

static inline void pte_free_kernel(struct mm_struct* mm, pte_t* pte)
{
    free_page((unsigned long)pte);
}

static inline void pte_free(struct mm_struct* mm, struct page* pte)
{
    pgtable_page_dtor(pte);
    __free_page(pte);
}


#define __pte_free_tlb(tlb, pte, addr)  \
do {                                    \
    pgtable_page_dtor(pte);             \
    tlb_remove_page((tlb), (pte));      \
} while (0)



#define pmd_pgtable(pmd)                pmd_page(pmd)

#define check_pgt_cache()               do { } while (0)

#endif
