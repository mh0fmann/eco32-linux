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
 * cpuinfo.h -- structure declaration for CPU information
 */

#ifndef __ASM_ECO32_CPUINFO_H
#define __ASM_ECO32_CPUINFO_H

struct cpuinfo {
	u32 clock_frequency;

	u32 icache_size;
	u32 icache_block_size;

	u32 dcache_size;
	u32 dcache_block_size;
};

extern struct cpuinfo cpuinfo;

#endif /* __ASM_ECO32_CPUINFO_H */
