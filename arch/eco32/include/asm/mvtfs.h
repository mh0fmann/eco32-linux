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

/*
 * On eco32 the cpu is mostly controled by the processor status word
 *
 * XXXX V UPO IPO IACK  MASK
 * 0000 0 000 000 00000 0000000000000000
 *
 * V    : Interrupt vector location. 0 means ROM, 1 means RAM
 * UPO  : Execution in usermode or previleged mode. 0 means previleged, 1 means usermode
 * IPO  : Interrupt acceptance status. 0 mean no interrupts, 1 means interrupts allowed
 * IACK : Current pending interrupt number
 * MASK : Interrupt enabled for specific interrupt number. 1 means not allowed, 1 means allowed
 */

/* Constants to access the special register in human readable way */

#define PSW             0
#define TLB_INDEX       1
#define TLB_ENTRY_HI    2
#define TLB_ENTRY_LO    3


/* Constants to access the PSW bits more easily */

#define CUM_SHIFT       26
#define PSW_CUM         (1 << CUM_SHIFT)
#define PUM_SHIFT       25
#define PSW_PUM         (1 << PUM_SHIFT)
#define OUM_SHIFT       24
#define PSW_OUM         (1 << OUM_SHIFT)
#define CIE_SHIFT       23
#define PSW_CIE         (1 << CIE_SHIFT)
#define PIE_SHIFT       22
#define PSW_PIE         (1 << PIE_SHIFT)
#define OIE_SHIFT       21
#define PSW_OIE         (1 << OIE_SHIFT)


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
