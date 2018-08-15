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
 * pm.c -- power management
 */

#include <linux/pm.h>
#include <linux/reboot.h>


#define SHUTDOWN_BASE	*((unsigned int*)0xFF100000)


void __noreturn eco32_restart(void);

void machine_power_off(void)
{
	pr_info("\nmachine power off");

	/* 
	 * the shutdown device isn't real and only present on the simulator
	 * so we need to catch that.
	 */
	SHUTDOWN_BASE = 0xBEDEAD;
	while (1);
}

void machine_halt(void)
{
	pr_info("\nmachine_halt requested\n");

	while (1);
}

void machine_restart(char* cmd)
{
	pr_info("\nmachine_restart: %s\n", cmd);
	eco32_restart();
}

void (*pm_power_off)(void) = machine_power_off;
