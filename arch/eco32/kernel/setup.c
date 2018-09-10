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
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/console.h>

#include <asm/setup.h>


#ifdef CONFIG_CMDLINE_BOOL
static const char eco32_builtin_cmdline[COMMAND_LINE_SIZE] __initdata = CONFIG_CMDLINE;
#endif


void __init setup_memory(void);
void setup_early_printk(void);


/*
 * eco32 early setup
 * this gets called from head.S befor we go to start_kernel
 * 
 * This is the last chance to setup stuff that needs to be set up
 * befor we go to start_kernel
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
	
	//we make the device tree ready befor we enter the kernel
	early_init_dt_scan(fdt);
	parse_early_param();
}


/*
 * setup arch related stuff
 */
void __init setup_arch(char** cmdline_p)
{
#ifdef CONFIG_EARLY_PRINTK
	setup_early_printk();
#endif

	unflatten_and_copy_device_tree();

	//setup memblock and all other eco32 related memory stuff
	setup_memory();

#ifdef CONFIG_CMDLINE_BOOL
	//copy builtin commandline
	strlcpy(boot_command_line, eco32_builtin_cmdline, COMMAND_LINE_SIZE);
#endif
	*cmdline_p = boot_command_line;
	pr_debug("initial thread info @ 0x%08lx\n",
	         (unsigned long) &init_thread_info);
	         
#if defined(CONFIG_VT) && defined(CONFIG_DUMMY_CONSOLE)
	//set dummy_con if we are going to use virtual terminals
	if (!conswitchp)
		conswitchp = &dummy_con;
#endif
}

