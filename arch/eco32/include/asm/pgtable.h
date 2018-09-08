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
 * pgtable.h -- macros and functions to manipulate page tables
 */

#ifndef __ASM_ECO32_PGTABLE_H
#define __ASM_ECO32_PGTABLE_H

#ifndef __ASSEMBLY__

#include <asm-generic/pgtable-nopmd.h>
#include <asm/mmu.h>
#include <asm/fixmap.h>
#include <asm/eco32.h>
#include <linux/mm_types.h>

/*
 * The Linux memory management assumes a more-level page table setup. On
 * ECO32, we use that, but "fold" the mid levels into the top-level page
 * table. Since the MMU TLB is software loaded through an interrupt, it
 * supports any page table structure, so we could have used a three-level
 * setup, but for the amounts of memory we normally use, a two-level is
 * probably more efficient.
 *
 * This file contains the functions and defines necessary to modify and use
 * the ECO32 page table tree.
 */

extern void paging_init(void);

/* Certain architectures need to do special things when pte's
 * within a page table are directly modified.  Thus, the following
 * hook is made available.
 */
#define set_pte(pteptr, pteval) ((*(pteptr)) = (pteval))
#define set_pte_at(mm, addr, ptep, pteval) set_pte(ptep, pteval)
/*
 * (pmds are folded into pgds so this doesn't get actually called,
 * but the define is needed for a generic inline function.)
 */
#define set_pmd(pmdptr, pmdval) (*(pmdptr) = pmdval)

#define PGDIR_SHIFT	(PAGE_SHIFT + (PAGE_SHIFT-2))
#define PGDIR_SIZE	(1UL << PGDIR_SHIFT)
#define PGDIR_MASK	(~(PGDIR_SIZE-1))

/*
 * entries per page directory level: we use a two-level, so
 * we don't really have any PMD directory physically.
 * pointers are 4 bytes so we can use the page size and
 * divide it by 4 (shift by 2).
 */
#define PTRS_PER_PTE	(1UL << (PAGE_SHIFT-2))
#define PTRS_PER_PGD	(1UL << (PAGE_SHIFT-2))

/*
 * calculate how many PGD entries a user-level program can use
 * the first mappable virtual address is 0
 * TASK_SIZE is the top of the virtual user address space
 */

#define USER_PTRS_PER_PGD       (512)
#define KRNL_PTRS_PER_PGD		(128)
#define FIRST_USER_ADDRESS      0UL

/*
 * Kernels own virtual memory area.
 */

/*
 * since our arch has fixed kernel mapped virtual area this range and size
 * is easy to set even if we will never be able to map 1GB of memory for the kernel
 * because we have way less memory.
 * 
 * NOTE: within this area we also have our fixmap placed at the start of the
 * kernel page mapped area and the pkmap at the end of that space.
 * so this may not be the complete 1GB in size
 */
#define VMALLOC_START		(ECO32_KERNEL_PAGE_MAPPED_START + FIXADDR_SIZE)
#ifdef CONFIG_HIGHMEM
/**
 * FIXME
 * 
 * NOTE: For some reason in include order we can not include asm/highmem.h in pgtable.h or we will get
 * errors. so we set that VMALLOC_END hardcoded to PKMAP_BASE. if PKMAP_BASE changes we need to
 * update that here as well.
 */
#define VMALLOC_END			(ECO32_KERNEL_DIRECT_MAPPED_RAM_START - PAGE_SIZE * PTRS_PER_PTE)
#else
#define VMALLOC_END			(ECO32_KERNEL_DIRECT_MAPPED_RAM_START)
#endif
#define VMALLOC_SIZE		(VMALLOC_END - VMALLOC_START)
#define VMALLOC_VMADDR(x)	((unsigned long)(x))


/*
 * An ECO32 PTE looks like this:
 *
 * |  31 ... 12  |  11 ... 6 |  5  |  4  |  3  |  2  |  1  |  0  |
 *  Frame number    reserved    S     A     D     U     W     P
 *
 *  S: Shared
 *  A: Accessed
 *  D: Dirty
 *  U: User page
 *  W: Write accessable (TLB Write Flag)
 *  P: Present (TLB Valid flag)
 *
 */

#define _PAGE_PRESENT	0x001
#define _PAGE_WRITE	0x002

#define _PAGE_USER	0x004
#define _PAGE_DIRTY	0x008
#define _PAGE_ACCESSED	0x010
#define _PAGE_SHARED	0x020


#define _PAGE_CHG_MASK	(PAGE_MASK | _PAGE_ACCESSED | _PAGE_DIRTY)
#define _PAGE_ALL       (_PAGE_PRESENT | _PAGE_ACCESSED)

#define PAGE_NONE       __pgprot(_PAGE_PRESENT | _PAGE_ACCESSED)
#define PAGE_READONLY   __pgprot(_PAGE_PRESENT | _PAGE_ACCESSED)
#define PAGE_SHARED 	__pgprot(_PAGE_PRESENT | _PAGE_WRITE)
#define PAGE_COPY       __pgprot(_PAGE_PRESENT)
#define PAGE_KERNEL 	__pgprot(_PAGE_PRESENT | _PAGE_ACCESSED \
		| _PAGE_DIRTY | _PAGE_WRITE | _PAGE_SHARED)
#define PAGE_KERNEL_NOCACHE (PAGE_KERNEL)

#define __P000	PAGE_NONE
#define __P001	PAGE_READONLY
#define __P010	PAGE_COPY
#define __P011	PAGE_COPY
#define __P100	PAGE_READONLY
#define __P101	PAGE_READONLY
#define __P110	PAGE_COPY
#define __P111	PAGE_COPY

#define __S000	PAGE_NONE
#define __S001	PAGE_READONLY
#define __S010	PAGE_SHARED
#define __S011	PAGE_SHARED
#define __S100	PAGE_READONLY
#define __S101	PAGE_READONLY
#define __S110	PAGE_SHARED
#define __S111	PAGE_SHARED


static inline int pte_write(pte_t pte)
{
	return pte_val(pte) & _PAGE_WRITE;
}
static inline int pte_dirty(pte_t pte)
{
	return pte_val(pte) & _PAGE_DIRTY;
}
static inline int pte_young(pte_t pte)
{
	return pte_val(pte) & _PAGE_ACCESSED;
}
static inline int pte_special(pte_t pte)
{
	return 0;
}

static inline pte_t pte_mkspecial(pte_t pte)
{
	return pte;
}
static inline pte_t pte_wrprotect(pte_t pte)
{
	pte_val(pte) &= ~(_PAGE_WRITE);
	return pte;
}
static inline pte_t pte_mkclean(pte_t pte)
{
	pte_val(pte) &= ~(_PAGE_DIRTY);
	return pte;
}
static inline pte_t pte_mkold(pte_t pte)
{
	pte_val(pte) &= ~(_PAGE_ACCESSED);
	return pte;
}
static inline pte_t pte_mkwrite(pte_t pte)
{
	pte_val(pte) |= _PAGE_WRITE;
	return pte;
}
static inline pte_t pte_mkdirty(pte_t pte)
{
	pte_val(pte) |= _PAGE_DIRTY;
	return pte;
}
static inline pte_t pte_mkyoung(pte_t pte)
{
	pte_val(pte) |= _PAGE_ACCESSED;
	return pte;
}

extern pgd_t swapper_pg_dir[PTRS_PER_PGD];

/*
 * ECO32 doesn't have any external MMU info: the kernel page
 * tables contain all the necessary information.
 */
static inline void update_mmu_cache(struct vm_area_struct* vma,
                                    unsigned long address, pte_t* pte)
{
}

/* zero page used for uninitialized stuff */
extern unsigned long empty_zero_page[1024];
#define ZERO_PAGE(vaddr) (virt_to_page(empty_zero_page))

/* number of bits that fit into a memory pointer */
#define BITS_PER_PTR			(8*sizeof(unsigned long))

/* to align the pointer to a pointer address */
#define PTR_MASK			(~(sizeof(void *)-1))

/* sizeof(void*)==1<<SIZEOF_PTR_LOG2 */
/* 64-bit machines, beware!  SRB. */
#define SIZEOF_PTR_LOG2			2

/* to find an entry in a page-table */
#define PAGE_PTR(address) \
	((unsigned long)(address)>>(PAGE_SHIFT-SIZEOF_PTR_LOG2)&PTR_MASK&~PAGE_MASK)


#define pte_none(x)	(!pte_val(x))
#define pte_present(x)	(pte_val(x) & _PAGE_PRESENT)
#define pte_clear(mm, addr, xp)	do { pte_val(*(xp)) = 0; } while (0)

#define pmd_none(x)	(!pmd_val(x))
#define	pmd_bad(x)	(pmd_val(x) & (~PAGE_MASK))
#define pmd_present(x)	(pmd_val(x))
#define pmd_clear(xp)	do { pmd_val(*(xp)) = 0; } while (0)



/*
 * Conversion functions: convert a page and protection to a page entry,
 * and a page entry and page directory to the page they refer to.
 */

/* What actually goes as arguments to the various functions is less than
 * obvious, but a rule of thumb is that struct page's goes as struct page *,
 * really physical DRAM addresses are unsigned long's, and DRAM "virtual"
 * addresses (the 0xc0xxxxxx's) goes as void *'s.
 */

static inline pte_t __mk_pte(void* page, pgprot_t pgprot)
{
	pte_t pte;
	/* the PTE needs a physical address */
	pte_val(pte) = __pa(page) | pgprot_val(pgprot);
	return pte;
}

#define mk_pte(page, pgprot) __mk_pte(page_address(page), (pgprot))

#define mk_pte_phys(physpage, pgprot) \
	({                                                                      \
	 pte_t __pte;                                                    \
	 \
	 pte_val(__pte) = (physpage) + pgprot_val(pgprot);               \
	 __pte;                                                          \
	 })

static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{
	pte_val(pte) = (pte_val(pte) & _PAGE_CHG_MASK) | pgprot_val(newprot);
	return pte;
}


/*
 * pte_val refers to a page in the 0x0xxxxxxx physical DRAM interval
 * __pte_page(pte_val) refers to the "virtual" DRAM interval
 * pte_pagenr refers to the page-number counted starting from the virtual
 * DRAM start
 */

static inline unsigned long __pte_page(pte_t pte)
{
	/* the PTE contains a physical address */
	return (unsigned long)__va(pte_val(pte) & PAGE_MASK);
}

#define pte_pagenr(pte)         ((__pte_page(pte) & ~PAGE_OFFSET) >> PAGE_SHIFT)

/* permanent address of a page */

#define __page_address(page) (PAGE_OFFSET | (((page) - mem_map) << PAGE_SHIFT))
#define pte_page(pte)		(mem_map+pte_pagenr(pte))

/*
 * only the pte's themselves need to point to physical DRAM (see above)
 * the pagetable links are purely handled within the kernel SW and thus
 * don't need the __pa and __va transformations.
 */
static inline void pmd_set(pmd_t* pmdp, pte_t* ptep)
{
	pmd_val(*pmdp) = (unsigned long) ptep;
}

#define pmd_page(pmd)		(pfn_to_page(__pa(pmd_val(pmd)) >> PAGE_SHIFT))
#define pmd_page_kernel(pmd)    ((unsigned long) __va(pmd_val(pmd) & PAGE_MASK))

/* to find an entry in a page-table-directory. */
#define pgd_index(address)      ((address >> PGDIR_SHIFT) & (PTRS_PER_PGD-1))

#define __pgd_offset(address)   pgd_index(address)

#define pgd_offset(mm, address) ((mm)->pgd+pgd_index(address))

/* to find an entry in a kernel page-table-directory */
#define pgd_offset_k(address) pgd_offset(&init_mm, address)

#define __pmd_offset(address) \
	(((address) >> PMD_SHIFT) & (PTRS_PER_PMD-1))

/*
 * the pte page can be thought of an array like this: pte_t[PTRS_PER_PTE]
 *
 * this macro returns the index of the entry in the pte page which would
 * control the given virtual address
 */
#define __pte_offset(address)                   \
	(((address) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))
#define pte_offset_kernel(dir, address)         \
	((pte_t *) pmd_page_kernel(*(dir)) +  __pte_offset(address))
#define pte_offset_map(dir, address)	        \
	((pte_t *)page_address(pmd_page(*(dir))) + __pte_offset(address))
#define pte_offset_map_nested(dir, address)     \
	pte_offset_map(dir, address)

#define pte_unmap(pte)          do { } while (0)
#define pte_unmap_nested(pte)   do { } while (0)
#define pte_pfn(x)		((unsigned long)(((x).pte)) >> PAGE_SHIFT)
#define pfn_pte(pfn, prot)  __pte((((pfn) << PAGE_SHIFT)) | pgprot_val(prot))

#define pte_ERROR(e) \
	pr_err("%s:%d: bad pte %p(%08lx).\n", \
			__FILE__, __LINE__, &(e), pte_val(e))
#define pgd_ERROR(e) \
	pr_err("%s:%d: bad pgd %p(%08lx).\n", \
			__FILE__, __LINE__, &(e), pgd_val(e))


/* __PHX__ FIXME, SWAP, this probably doesn't work */

/* Encode and de-code a swap entry (must be !pte_none(e) && !pte_present(e)) */
/* Since the PAGE_PRESENT bit is bit 4, we can use the bits above */

#define __swp_type(x)			(((x).val >> 5) & 0x7f)
#define __swp_offset(x)			((x).val >> 12)
#define __swp_entry(type, offset) \
	((swp_entry_t) { ((type) << 5) | ((offset) << 12) })
#define __pte_to_swp_entry(pte)		((swp_entry_t) { pte_val(pte) })
#define __swp_entry_to_pte(x)		((pte_t) { (x).val })

#define kern_addr_valid(addr)           (1)

#include <asm-generic/pgtable.h>

/*
 * No page table caches to initialise
 */
#define pgtable_cache_init()		do { } while (0)

typedef pte_t* pte_addr_t;

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ECO32_PGTABLE_H */
