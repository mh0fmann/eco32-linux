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
 * io.h -- remap of io devices
 */

#ifndef __ASM_ECO32_IO_H
#define __ASM_ECO32_IO_H

#include <asm-generic/io.h>
#include <asm/pgtable.h>

void __iomem* __ioremap(phys_addr_t offset, unsigned long size);

static inline void __iomem* ioremap(phys_addr_t offset,
                                    unsigned long size)
{
	return __ioremap(offset, size);
}

static inline void __iomem* ioremap_nocache(phys_addr_t offset,
        unsigned long size)
{
	return __ioremap(offset, size);
}

void iounmap(void* addr);

#endif
