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
 * init.c -- initialize the memory management
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/memblock.h>
#include <linux/sched.h>
#include <linux/mmzone.h>

#include <asm/page.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/tlb.h>
#include <asm/sections.h>
#include <asm/highmem.h>
#include <asm/fixmap.h>



static void __init zones_size_init(void)
{
	unsigned long zones_size[MAX_NR_ZONES];
	memset(zones_size, 0x00, sizeof(zones_size));
	zones_size[ZONE_NORMAL] = max_pfn;
#ifdef CONFIG_HIGHMEM
	zones_size[ZONE_HIGHMEM] = highend_pfn;
#endif

	/* initialize the free area and get the allocators up and running */
	free_area_init(zones_size);
}


void __init paging_init(void)
{
	memset(swapper_pg_dir, 0x00, sizeof(swapper_pg_dir));

	current_pgd = init_mm.pgd;

	/* init zones and mem_map */
	zones_size_init();
	
	/*
	 * init fixmap area and kmap area
	 */
	fixmap_init();
#ifdef CONFIG_HIGHMEM
	kmap_init();
#endif

	/* 
	 * from now on tlb exceptions can happen and we should be able
	 * to catch them and handle them
	 */
	set_tlb_handler();

	/* flush all tlb entires */
	flush_tlb_all();
}


/* void __init mem_init(void)
 *
 * Do last stepts of memory init.
 * Set high memory and free all memory which is no longer needed
 */
void __init mem_init(void)
{
	/* everything after direct mapped ram is highmem */
	high_memory = (void*)ECO32_KERNEL_DIRECT_MAPPED_RAM_SIZE;

	/* empty the zero page */
	memset((void*)empty_zero_page, 0x00, PAGE_SIZE);

	/* put low memory on the freelist */
	free_all_bootmem();

	mem_init_print_info(NULL);

	pr_info("Virtual kernel memory layout:\n");
	pr_cont("     lowmem : 0x%08lx - 0x%08lx   (%4ld MB)\n",
	        (unsigned long)__va(min_low_pfn),
	        (unsigned long)__va(max_low_pfn),
	        ((unsigned long)__va(max_low_pfn) -
	         (unsigned long)__va(0)) >> 20);
#ifdef CONFIG_HIGHMEM
	pr_cont("      pkmap : 0x%08lx - 0x%08lx   (%4ld KB)\n",
	        PKMAP_BASE, PKMAP_BASE+LAST_PKMAP*PAGE_SIZE,
	        (LAST_PKMAP*PAGE_SIZE) >> 10);
#endif
	pr_cont("    vmalloc : 0x%08lx - 0x%08lx   (%4ld MB)\n",
	        VMALLOC_START, VMALLOC_END,
	        (VMALLOC_END - VMALLOC_START) >> 20);
	pr_cont("     fixmap : 0x%08lx - 0x%08lx   (%4d kB)\n",
	        FIXADDR_START, FIXADDR_TOP, FIXADDR_SIZE >> 10);
	pr_cont("      .text : 0x%08lx - 0x%08lx   (%4ld kB)\n",
	        (unsigned long)&_stext, (unsigned long)&_etext,
	        ((unsigned long)&_etext - (unsigned long)&_stext) >> 10);
	pr_cont("      .data : 0x%08lx - 0x%08lx   (%4ld kB)\n",
	        (unsigned long)&_etext, (unsigned long)&_edata,
	        ((unsigned long)&_edata - (unsigned long)&_etext) >> 10);
	pr_cont("      .init : 0x%08lx - 0x%08lx   (%4ld kB)\n",
	        (unsigned long)&__init_begin,
	        (unsigned long)&__init_end,
	        ((unsigned long)&__init_end -
	         (unsigned long)&__init_begin) >> 10);
}


/*
 * void free_initrd_mem(unsigned long start, unsigned long end)
 */
#ifdef CONFIG_BLK_DEV_INITRD
void free_initrd_mem(unsigned long start, unsigned long end)
{
	free_reserved_area((void*)start, (void*)end, -1, "initrd");
}
#endif


/* void free_initmem(void)
 *
 * free the memory with the __init marked code
 */
void free_initmem(void)
{
	free_initmem_default(-1);
}
