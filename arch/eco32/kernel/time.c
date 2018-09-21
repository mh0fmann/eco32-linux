/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * time.c -- the ECO32 timer intialization
 */


#include <linux/clocksource.h>


void __init time_init(void)
{
	/*
	 * Timer initialization is all done int the timer driver
	 */
	timer_probe();
}
