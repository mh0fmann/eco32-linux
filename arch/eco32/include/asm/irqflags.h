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


static inline unsigned long arch_local_save_flags(void)
{
    return __eco32_read_psw() & ARCH_IRQ_ENABLED;
}


static inline void arch_local_irq_restore(unsigned long flags)
{
    unsigned long psw = __eco32_read_psw();
    __eco32_write_psw(psw | flags);
}

#include <asm-generic/irqflags.h>

#endif /* __ASM__ECO32_IRQFLAGS_H */
