/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2017 Hellwig Geisse
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * delay.c -- precise delay loops
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/processor.h>


void __delay(unsigned long cycles);


void __const_udelay(unsigned long xloops)
{
	unsigned long long loops;

	loops = (unsigned long long) xloops * loops_per_jiffy * HZ;
	__delay(loops >> 32);
}


void __udelay(unsigned long usecs)
{
	/* multiplier = 2^32 / 10^6, rounded up */
	__const_udelay(usecs * 4295UL);
}


void __ndelay(unsigned long nsecs)
{
	/* multiplier = 2^32 / 10^9, rounded up */
	__const_udelay(nsecs * 5UL);
}
