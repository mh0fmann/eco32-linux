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

#ifndef __ASM_ECO32_IRQFLAGS_H
#define __ASM_ECO32_IRQFLAGS_H

#include <asm/mvtfs.h>

#define ARCH_IRQ_DISABLED   0
#define ARCH_IRQ_ENABLED    PSW_CIE


extern unsigned long psw;

static inline unsigned long arch_local_save_flags(void)
{
    return psw & ARCH_IRQ_ENABLED;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
    __eco32_write_psw(0);
    if (flags) {
        psw |= flags;
    } else {
        psw &= ~ARCH_IRQ_ENABLED;
    }
    __eco32_write_psw(psw);
}

#define arch_local_irq_save arch_local_irq_save
static inline unsigned long arch_local_irq_save(void)
{
    unsigned long flags;
    __eco32_write_psw(0);
    flags = psw & ARCH_IRQ_ENABLED;
    psw &= ~ARCH_IRQ_ENABLED;
    __eco32_write_psw(psw);
    return flags;
}

#include <asm-generic/irqflags.h>

#endif /* __ASM__ECO32_IRQFLAGS_H */
