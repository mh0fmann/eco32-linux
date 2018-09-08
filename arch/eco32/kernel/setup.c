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
 * setup.c -- arch specific kernel setup
 */

#include <linux/string.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/sched.h>
#include <linux/pfn.h>
#include <linux/bootmem.h>
#include <linux/memblock.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#include <asm/page.h>
#include <asm/sections.h>
#include <asm/pgtable.h>
#include <asm/irq.h>
#include <asm/cpuinfo.h>
#include <asm/setup.h>
#include <asm/eco32.h>

#include <linux/seq_file.h>
#include <linux/delay.h>


/**************************************************************/


#ifdef CONFIG_CMDLINE_BOOL
static const char eco32_builtin_cmdline[COMMAND_LINE_SIZE] __initdata = CONFIG_CMDLINE;
#endif


/*
 * Initialize device tree handling.
 */
void __init early_init_devtree(void* params)
{
	early_init_dt_scan(params);
	parse_early_param();
	memblock_allow_resize();
}


/*
 * Initialize device tree handling.
 * Fall back on built-in device tree
 * in case the argument is NULL.
 */
void __init eco32_early_setup(void* fdt)
{
	if (fdt != NULL) {
		pr_debug("External FDT at 0x%08lX\n",
		        (unsigned long) fdt);
	} else {
		fdt = __dtb_start;
		pr_debug("Internal FDT at 0x%08lX\n",
		        (unsigned long) fdt);
	}

	early_init_devtree(fdt);
}


/**************************************************************/


static int __init eco32_device_probe(void)
{
	of_platform_populate(NULL, NULL, NULL, NULL);
	return 0;
}


device_initcall(eco32_device_probe);


/**************************************************************/


struct cpuinfo cpuinfo;


static void print_cpuinfo(void)
{
	pr_info("CPU: ECO32\n");
}


void __init setup_cpuinfo(void)
{
	struct device_node* cpu;

	cpu = of_find_compatible_node(NULL, NULL, "thm,eco32-cpu");

	if (cpu == NULL) {
		panic("no compatible CPU found in device tree");
	}

	of_node_put(cpu);
	print_cpuinfo();
}


/**************************************************************/


/* static void __init setup_memory(void)
 *
 * Sets the memory ranges for memblock.
 */
static void __init setup_memory(void)
{
	int num_regions;
	struct memblock_region* region;
	phys_addr_t memory_start = 0;
	phys_addr_t memory_end = 0;
	unsigned long ram_start_pfn;
	unsigned long ram_end_pfn;

	/* find main memory */
	num_regions = 0;
	for_each_memblock(memory, region) {
		memory_start = region->base;
		memory_end = region->base + region->size;
		num_regions++;
	}

	if (num_regions < 1) {
		panic("no memory regions");
	}

	if (num_regions > 1) {
		panic("more than one memory region");
	}

	if (memory_end <= memory_start) {
		panic("no memory");
	}

	ram_start_pfn = PFN_UP(memory_start);
	ram_end_pfn = PFN_DOWN(memblock_end_of_DRAM());

	min_low_pfn = ram_start_pfn;
	max_low_pfn = PFN_DOWN(__pa(ECO32_KERNEL_DIRECT_MAPPED_RAM_END));
	
	max_pfn = ram_end_pfn;

	/* reserver memory */
	memblock_reserve(__pa(_stext), _end - _stext);
	memblock_reserve(__pa(ECO32_KERNEL_DIRECT_MAPPED_ROM_START),
					 __pa(ECO32_KERNEL_DIRECT_MAPPED_IO_END));
	early_init_fdt_reserve_self();
	early_init_fdt_scan_reserved_mem();
	__memblock_dump_all();
}


/**************************************************************/


void setup_early_printk(void);


void __init setup_arch(char** cmdline_p)
{
#ifdef CONFIG_EARLY_PRINTK
	setup_early_printk();
#endif
	unflatten_and_copy_device_tree();
	setup_cpuinfo();
	/* initial memory region is kernel code/data */
	init_mm.start_code = (unsigned long) _stext;
	init_mm.end_code = (unsigned long) _etext;
	init_mm.end_data = (unsigned long) _edata;
	init_mm.brk = (unsigned long) _end;
	/* setup memblock allocator */
	setup_memory();
	/* setup MMU and mark all pages as reserved */
	paging_init();
#ifdef CONFIG_CMDLINE_BOOL /* use builtin cmdline */
	strlcpy(boot_command_line, eco32_builtin_cmdline, COMMAND_LINE_SIZE);
#endif
	*cmdline_p = boot_command_line;
	pr_debug("initial thread info @ 0x%08lx\n",
	         (unsigned long) &init_thread_info);
#if defined(CONFIG_VT) && defined(CONFIG_DUMMY_CONSOLE)
	if (!conswitchp)
		conswitchp = &dummy_con;
#endif
}


/**************************************************************/


static void* c_start(struct seq_file* m, loff_t* pos)
{
	/* we only have one CPU */
	return *pos < 1 ? (void*) 1 : NULL;
}


static void* c_next(struct seq_file* m, void* v, loff_t* pos)
{
	++*pos;
	return NULL;
}


static void c_stop(struct seq_file* m, void* v)
{
}


static int show_cpuinfo(struct seq_file* m, void* v)
{
	seq_printf(m,
	           "cpu\t\t: ECO32\n"
	           "frequency\t: %ld\n"
	           "bogomips\t: %lu.%02lu\n",
	           loops_per_jiffy * HZ,
	           (loops_per_jiffy * HZ) / 500000,
	           ((loops_per_jiffy * HZ) / 5000) % 100);
	return 0;
}


const struct seq_operations cpuinfo_op = {
	.start = c_start,
	.next = c_next,
	.stop = c_stop,
	.show = show_cpuinfo,
};
