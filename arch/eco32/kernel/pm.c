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


#include <linux/pm.h>
#include <linux/reboot.h>

#include <asm/mvtfs.h>
#include <asm/eco32.h>


#define SHUTDOWN_BASE   *((unsigned int*)0xFF100000)


void machine_power_off(void)
{
    pr_info("\nmachine power off");

#ifdef CONFIG_ECO32_SIMULATOR
    /* 
     * the shutdown device isn't real and only present on the simulator
     * so we need to catch that.
     */
    SHUTDOWN_BASE = 0xBEDEAD;
#else
    while (1);
#endif
}

void machine_halt(void)
{
    pr_info("\nmachine_halt requested\n");

    while (1);
}

void machine_restart(char* cmd)
{
    unsigned int rom = ECO32_KERNEL_DIRECT_MAPPED_ROM_START;
    pr_info("\nmachine_restart: %s\n", cmd);
    __eco32_write_psw(0);
    __asm__ volatile("jalr   %0" : "=r"(rom));
}

void (*pm_power_off)(void) = machine_power_off;
