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

#include <asm/io.h>


void __iomem* __ioremap(phys_addr_t addr, unsigned long size)
{
	unsigned long last_addr;

	/* don't allow wraparound or zero size */
	last_addr = addr + size - 1;

	if (size == 0 || last_addr < addr) {
		return NULL;
	}

	/* don't allow addresses outside of physical I/O range */
	if (addr < 0x30000000 || addr >= 0x40000000) {
		return NULL;
	}

	/* direct-mapped addresses are easy to translate */
	return (void __iomem*) (0xC0000000 | addr);
}
