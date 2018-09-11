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
#include <linux/mmzone.h>
#include <linux/of_fdt.h>

#include <asm/page.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/tlb.h>
#include <asm/sections.h>


static void __init zones_size_init(void)
{
	//on eco32 we only habe ZONE_NORMAL
	//there is no dma or highmem atm.
	unsigned long zones_size[MAX_NR_ZONES];
	memset(zones_size, 0x00, sizeof(zones_size));
	zones_size[ZONE_NORMAL] = max_pfn;

	//initialize the free area and get the allocators up and running
	free_area_init(zones_size);
}


/*
 * setup arch specific memory
 */
void __init setup_memory(void){
	
	//initial memory region is kernel code/data
	init_mm.start_code = (unsigned long) _stext;
	init_mm.end_code = (unsigned long) _etext;
	init_mm.end_data = (unsigned long) _edata;
	init_mm.brk = (unsigned long) _end;
	
	//on eco32 memory always starts on 0x00000000
	//due to our mmu desing we always map the first 512mb of ram
	//this is our lowmem
	min_low_pfn = PFN_UP(0x00000000);
	max_low_pfn = PFN_DOWN(__pa(ECO32_KERNEL_DIRECT_MAPPED_RAM_END));
	max_pfn = PFN_DOWN(memblock_end_of_DRAM());

	memblock_allow_resize();
	
	//reserve memory regions
	memblock_reserve(__pa(_stext), _end - _stext);
	memblock_reserve(__pa(ECO32_KERNEL_DIRECT_MAPPED_ROM_START),
					 __pa(ECO32_KERNEL_DIRECT_MAPPED_IO_END));	 
	early_init_fdt_reserve_self();
	early_init_fdt_scan_reserved_mem();
	
	__memblock_dump_all();
	
	zones_size_init();
	
	//initialize swapper and empty_zero_page
	memset(swapper_pg_dir, 0x00, PAGE_SIZE);
	memset(empty_zero_page, 0x00, PAGE_SIZE);
	
	//let current_pgd point to something sane
	current_pgd = init_mm.pgd;
	
	//set the tlb handler
	set_tlb_handler();
}


void __init mem_init(void)
{
	mem_init_print_info(NULL);
	
	pr_info("Virtual kernel memory layout:\n");
	pr_cont("     lowmem : 0x%08lx - 0x%08lx   (%4ld MB)\n",
	        (unsigned long)ECO32_KERNEL_DIRECT_MAPPED_RAM_START,
	        (unsigned long)ECO32_KERNEL_DIRECT_MAPPED_RAM_END,
	        ((unsigned long)ECO32_KERNEL_DIRECT_MAPPED_RAM_START -
	         (unsigned long)ECO32_KERNEL_DIRECT_MAPPED_RAM_END) >> 20);
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

	//put remaining bootmem on the free list
	free_all_bootmem();
}


#ifdef CONFIG_BLK_DEV_INITRD
void free_initrd_mem(unsigned long start, unsigned long end)
{
	free_reserved_area((void*)start, (void*)end, -1, "initrd");
}
#endif


void free_initmem(void)
{
	free_initmem_default(-1);
}
