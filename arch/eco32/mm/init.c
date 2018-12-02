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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/memblock.h>
#include <linux/mmzone.h>
#include <linux/of_fdt.h>
#include <linux/initrd.h>

#include <asm/page.h>
#include <asm/mmu_context.h>
#include <asm/pgtable.h>
#include <asm/tlb.h>
#include <asm/sections.h>


/*
 * Pointe to the current_pgd which holds the pgd of the current
 * running process
 */
volatile pgd_t* current_pgd = swapper_pg_dir;


void __init early_init_dt_add_memory_arch(u64 base, u64 size)
{
    /* only add memory that is within the valid memory range of the eco32 */
    if ((base < __pa(ECO32_KERNEL_DIRECT_MAPPED_RAM_END)) &&
        (base + size < __pa(ECO32_KERNEL_DIRECT_MAPPED_RAM_END))) {
        memblock_add(base, size);
    }
}

int __init early_init_dt_reserve_memory_arch(phys_addr_t base,
                                             phys_addr_t size,
                                             bool nomap)
{
    if (nomap)
        return memblock_remove(base, size);
    /* some addresses could be within the DIRECT_MAPPED_SPACE
     * translate those to physical ones
     */
    base = __pa(base);
    return memblock_reserve(base, size);
}


/*
 * setup arch specific memory
 */
void __init setup_arch_memory(void)
{
    unsigned long zones_size[MAX_NR_ZONES];

    /* initial memory region is kernel code/data */
    init_mm.start_code = (unsigned long) _text;
    init_mm.end_code = (unsigned long) _etext;
    init_mm.end_data = (unsigned long) _edata;
    init_mm.brk = (unsigned long) _end;

    /* --------- set lowmem pageframe numbers ----------
     *
     * on eco32 we only use lowmem due to our current architecture
     * design.
     * it is always the same in size and within the same addresses
     * our mmu always covers this range for for priviliged access
     */

    min_low_pfn = PFN_UP(0x00000000);
    max_low_pfn = PFN_DOWN(__pa(ECO32_KERNEL_DIRECT_MAPPED_RAM_END));
    max_pfn = PFN_DOWN(memblock_end_of_DRAM());


    /* ------- memblock bootime allocator setup -------
     *
     * memory regions allready aded by the early device tree scan
     *
     * we will still need to mark some regions as reserved.
     * this includes:
     * - kernel text and data segments
     * - ram and rom regions. (just to be save)
     * - initrd
     * - device tree itself
     * - reserved areas from the device tree
     */

    memblock_allow_resize();

    /* reserve memory regions */
    memblock_reserve(__pa(_text), _end - _text);
    memblock_reserve(__pa(ECO32_KERNEL_DIRECT_MAPPED_ROM_START),
                     __pa(ECO32_KERNEL_DIRECT_MAPPED_IO_END));
#ifdef CONFIG_BLK_DEV_INITRD
    memblock_reserve(__pa(initrd_start), __pa(initrd_end - initrd_start));
#endif

    early_init_fdt_reserve_self();
    early_init_fdt_scan_reserved_mem();

    __memblock_dump_all();


    /* ----------------- init zones ------------------
     *
     * on eco32 we only have ZONE_NORMAL. there is no highmem
     * or dma atm.
     */
    memset(zones_size, 0x00, sizeof(zones_size));
    zones_size[ZONE_NORMAL] = max_pfn;

    /* initialize the free area and get the allocators up and running */
    free_area_init(zones_size);


    /* ---------- init swapper and zero page -----------
     *
     * on eco32 we only have ZONE_NORMAL. there is no highmem
     * or dma atm.
     */
    memset(swapper_pg_dir, 0x00, PAGE_SIZE);
    memset(empty_zero_page, 0x00, PAGE_SIZE);

    /* set the tlb handler */
    set_tlb_handler();
}


void __init mem_init(void)
{
    mem_init_print_info(NULL);

    pr_info("Virtual kernel memory layout:\n");
    pr_cont("     lowmem : 0x%08x - 0x%08x   (%4ld MB)\n",
            ECO32_KERNEL_DIRECT_MAPPED_RAM_START,
            ECO32_KERNEL_DIRECT_MAPPED_RAM_END,
            (unsigned long)(ECO32_KERNEL_DIRECT_MAPPED_RAM_START -
             ECO32_KERNEL_DIRECT_MAPPED_RAM_END) >> 20);
    pr_cont("    vmalloc : 0x%08x - 0x%08x   (%4ld MB)\n",
            VMALLOC_START, VMALLOC_END,
            (unsigned long)(VMALLOC_END - VMALLOC_START) >> 20);
    pr_cont("     fixmap : 0x%08lx - 0x%08lx   (%4ld kB)\n",
            FIXADDR_START, FIXADDR_TOP,
            (unsigned long)FIXADDR_SIZE >> 10);
    pr_cont("      .text : 0x%08x - 0x%08x   (%4ld kB)\n",
            (int)&_stext, (int)&_etext,
            (unsigned long)((int)&_etext - (int)&_stext) >> 10);
    pr_cont("      .data : 0x%08x - 0x%08x   (%4ld kB)\n",
            (int)&_etext, (int)&_edata,
            ((unsigned long)&_edata - (int)&_etext) >> 10);
    pr_cont("      .init : 0x%08x - 0x%08x   (%4ld kB)\n",
            (int)&__init_begin,
            (int)&__init_end,
            ((int)&__init_end -
             (unsigned long)&__init_begin) >> 10);

    /* put remaining bootmem on the free list */
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
