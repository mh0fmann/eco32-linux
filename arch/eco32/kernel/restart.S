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
 * restart.S -- restarting the machine
 */


/*
 * void __noreturn eco32_restart(void)
 */
	.text
	.global eco32_restart
eco32_restart:
	mvts $0,0
	ori $4,$0,0xE0000000
	jalr $4
	
