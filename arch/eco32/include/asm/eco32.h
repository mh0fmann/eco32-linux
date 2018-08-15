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
 * eco32.h -- ECO32 address boundaries and inline assamble functions
 */

#ifndef __ASM_ECO32_H
#define __ASM_ECO32_H


#define ECO32_USER_PAGE_MAPPED_START			0x00000000
#define ECO32_USER_PAGE_MAPPED_SIZE				0x80000000
#define ECO32_KERNEL_PAGE_MAPPED_START			0x80000000
#define ECO32_KERNEL_PAGE_MAPPED_SIZE			0x40000000
#define ECO32_KERNEL_DIRECT_MAPPED_RAM_START	0xC0000000
#define ECO32_KERNEL_DIRECT_MAPPED_RAM_SIZE		0x20000000
#define ECO32_KERNEL_DIRECT_MAPPED_ROM_START	0xE0000000
#define ECO32_KERNEL_DIRECT_MAPPED_ROM_SIZE		0x10000000
#define ECO32_KERNEL_DIRECT_MAPPED_IO_START		0xF0000000
#define ECO32_KERNEL_DIRECT_MAPPED_IO_SIZE		0x10000000


#define __eco32_write_tlbi()	__asm__("tbwi")
#define __eco32_write_tlbr()	__asm__("tbwr")
#define __eco32_read_tlbi()		__asm__("tbri")
#define __eco32_search_tlb()	__asm__("tbs")


static inline unsigned long __eco32_read_tlbidx(void)
{
	unsigned long idx;
	__asm__("mvfs %0, 1" : "=r" (idx));
	return idx;
}

static inline void __eco32_write_tlbidx(unsigned int idx)
{
	__asm__("mvts %0, 1" :: "r"(idx));
}

static inline unsigned long __eco32_read_tlbhi(void)
{
	unsigned long addr;
	__asm__("mvfs %0, 2" : "=r" (addr));
	return addr;
}

static inline void __eco32_write_tlbhi(unsigned long hi)
{
	__asm__("mvts %0, 2" :: "r"(hi));
}

static inline unsigned long __eco32_read_tlblo(void)
{
	unsigned long addr;
	__asm__("mvfs %0, 3" : "=r" (addr));
	return addr;
}

static inline void __eco32_write_tlblo(unsigned long lo)
{
	__asm__("mvts %0, 3" :: "r"(lo));
}

static inline unsigned long __eco32_read_tlbbad(void)
{
	unsigned long addr;
	__asm__("mvfs %0, 4" : "=r" (addr));
	return addr; 
}

#endif
