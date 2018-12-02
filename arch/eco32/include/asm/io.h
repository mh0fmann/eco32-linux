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

#ifndef __ASM_ECO32_IO_H
#define __ASM_ECO32_IO_H


#include <linux/types.h>

void __iomem* __ioremap(phys_addr_t offset, unsigned long size);

#define ioremap(offset, size)           __ioremap(offset, size)
#define ioremap_nocache(offset, size)   __ioremap(offset, size)

#define iounmap(addr)                   do { } while(0);

#include <asm-generic/io.h>

#endif
