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
 * fixmap.c - initialize the fixmap area
 */


#include <asm/fixmap.h>
#include <asm/pgtable.h>
#include <linux/memblock.h>
#include <linux/string.h>
#include <linux/kernel.h>


void __init fixmap_init(void)
{
	pgd_t* pgd;
	unsigned long fixmap_pte;
	
	if (FIXADDR_SIZE <= 0 ) {
		return;
	}
	
	fixmap_pte = memblock_alloc(PAGE_SIZE, PAGE_SIZE);
	memset(__va(fixmap_pte), 0x00, PAGE_SIZE);
	
	pgd = pgd_offset_k(FIXADDR_START);
	pgd->pgd = (unsigned long)__va(fixmap_pte);
}

