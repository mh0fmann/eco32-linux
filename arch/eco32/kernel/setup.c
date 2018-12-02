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

#include <linux/string.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/console.h>
#include <linux/initrd.h>

#include <asm/setup.h>


void __init setup_arch_memory(void);
void __init setup_early_printk(void);


/*
 * eco32 early setup
 * this gets called from head.S befor we go to start_kernel
 *
 * This is the last chance to setup stuff that needs to be set up
 * befor we go to start_kernel
 */
void __init eco32_early_setup(char* cmdline, void* fdt,
                              unsigned int initrds,
                              unsigned int initrde)
{
    if (IS_ENABLED(CONFIG_ARGUMENT_DEVICETREE) && fdt) {
        /* we make the device tree ready befor we enter the kernel */
        early_init_dt_scan(fdt);
        parse_early_param();
    } else if (IS_ENABLED(CONFIG_BUILTIN_DTB)) {
        early_init_dt_scan(__dtb_start);
        parse_early_param();
    }

#ifdef CONFIG_ARGUMENT_CMDLINE
    if (cmdline) {
        strncpy(boot_command_line, cmdline, COMMAND_LINE_SIZE);
    }
#endif

#ifdef CONFIG_ARGUMENT_INITRD
    if (initrds && initrde) {
        initrd_start = initrds;
        initrd_end = initrde;
    }
#endif
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

    /* setup memblock and all other eco32 related memory stuff */
    setup_arch_memory();

    *cmdline_p = boot_command_line;
    pr_debug("initial thread info @ 0x%08lx\n",
             (unsigned long) &init_thread_info);

#if defined(CONFIG_VT) && defined(CONFIG_DUMMY_CONSOLE)
    /* use dummy_con if we are going to use virtual terminals */
    if (!conswitchp)
        conswitchp = &dummy_con;
#endif
}

