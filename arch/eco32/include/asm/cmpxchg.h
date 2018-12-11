/*
 * Linux architectural port borrowing liberally from similar works of
 * others, namely OpenRISC and RISC-V.  All original copyrights apply
 * as per the original source declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef __ASM_ECO32_CMPXCHG_H
#define __ASM_ECO32_CMPXCHG_H

#include <linux/build_bug.h>
#include <linux/irqflags.h>



/* On eco32 we only do 32bit atomic operations */

#define __xchg_u32    ___xchg_u32
static inline unsigned long ___xchg_u32(unsigned long new,
                                        volatile void* ptr)
{
    unsigned long ret;

    __asm__ __volatile__(
        "0: ldlw    %0,%2,0     \n"
        "   ori     %1,%3,0     \n"
        "   stcw    %1,%2,0     \n"
        "   beq     %1,$0,0b    \n"
        : "=&r"(ret), "+&r"(new)
        : "r"(ptr), "r"(new)
        : "cc", "memory");

    return ret;
}


static inline unsigned long ___cmpxchg_u32(unsigned long old,
                                           unsigned long new,
                                           volatile void* ptr)
{
    unsigned long ret;

    __asm__ __volatile__(
        "0: ldlw    %0,%2,0     \n"
        "   bne     %0,%3,1f    \n"
        "   ori     %1,%4,0     \n"
        "   stcw    %1,%2,0     \n"
        "   beq     %1,$0,0b    \n"
        "1:                     \n"
        : "=&r"(ret), "+&r"(new)
        : "r"(ptr), "r"(old), "r"(new)
        : "cc", "memory");

    return ret;
}


static inline unsigned long __cmpxchg_local(volatile void *ptr,
                                            unsigned long old,
                                            unsigned long new,
                                            int size)
{
    /*
     * Sanity checking, compile-time.
     */
    if (size == 8 && sizeof(unsigned long) != 8)
        BUILD_BUG();


    if (size == 4) {
        return ___cmpxchg_u32(old, new, ptr);
    } else {
        unsigned long flags, prev;

        raw_local_irq_save(flags);
        switch (size) {
        case 1: prev = *(u8 *)ptr;
            if (prev == old)
                *(u8 *)ptr = (u8)new;
            break;
        case 2: prev = *(u16 *)ptr;
            if (prev == old)
                *(u16 *)ptr = (u16)new;
            break;
        case 8: prev = *(u64 *)ptr;
            if (prev == old)
                *(u64 *)ptr = (u64)new;
            break;
        default:
            BUILD_BUG();
        }
        raw_local_irq_restore(flags);
        return prev;
    }
}

#define cmpxchg_local(ptr, o, n)                                    \
({                                                                  \
    (__typeof__(*(ptr))) __cmpxchg_local((ptr),                     \
                                         (unsigned long)(o),        \
                                         (unsigned long)(n),        \
                                         sizeof(*(ptr)));           \
})

#include <asm-generic/cmpxchg.h>

#endif /* __ASM_ECO32_CMPXCHG_H */
