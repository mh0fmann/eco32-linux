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


#ifndef __ASM_MVTFS_H
#define __ASM_MVTFS_H


#define PSW             0
#define TLB_INDEX       1
#define TLB_ENTRY_HI    2
#define TLB_ENTRY_LO    3


#ifndef __ASSEMBLY__

/*
 * Makros and helpers function to read and manipulate
 * the psw from C sources
 */
static inline unsigned long __eco32_read_psw(void)
{
    unsigned long psw;
    __asm__("mvfs %0, 0": "=r" (psw));
    return psw;
}

static inline void __eco32_write_psw(unsigned long psw)
{
    __asm__("mvts %0, 0" :: "r" (psw));
}

/*
 * Makros and helper functions to read and manipulate
 * the tlb from C sources
 */
#define __eco32_write_tlbi()    __asm__("tbwi")
#define __eco32_write_tlbr()    __asm__("tbwr")
#define __eco32_read_tlbi()     __asm__("tbri")
#define __eco32_search_tlb()    __asm__("tbs")


static inline unsigned long __eco32_read_tlbidx(void)
{
    unsigned long idx;
    __asm__("mvfs %0, 1" : "=r" (idx));
    return idx;
}

static inline void __eco32_write_tlbidx(unsigned long idx)
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

#endif /* __ASSEMBLY__ */

#endif /* __ASM_MVTFS_H */
