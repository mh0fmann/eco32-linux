/*
 * Linux architectural port borrowing liberally from similar works of
 * others, namely OpenRISC and RISC-V.  All original copyrights apply
 * as per the original source declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Hellwig Geisse
 * Copyright (c) 2019 Martin Hofmann
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


extern volatile unsigned long psw;


static inline unsigned long arch_local_save_flags(void)
{
    unsigned long flags;
    __eco32_write_psw(0);
    flags = psw & ARCH_IRQ_ENABLED;
    __eco32_write_psw(psw);
    return flags;
}


static inline void arch_local_irq_restore(unsigned long flags)
{
    __eco32_write_psw(0);
    psw |= (flags & ARCH_IRQ_ENABLED);
    __eco32_write_psw(psw);
}


static inline unsigned long arch_local_irq_save(void)
{
    unsigned long flags;
    __eco32_write_psw(0);
    flags = psw & ARCH_IRQ_ENABLED;
    psw &= ~ARCH_IRQ_ENABLED;
    __eco32_write_psw(psw);
    return flags;
}


static inline int arch_irqs_disabled_flags(unsigned long flags)
{
    return flags == ARCH_IRQ_DISABLED;
}


static inline void arch_local_irq_enable(void)
{
    __eco32_write_psw(0);
    psw |= ARCH_IRQ_ENABLED;
    __eco32_write_psw(psw);
}


static inline void arch_local_irq_disable(void)
{
    __eco32_write_psw(0);
    psw &= ~ARCH_IRQ_ENABLED;
    __eco32_write_psw(psw);
}


static inline int arch_irqs_disabled(void)
{
    return arch_irqs_disabled_flags(arch_local_save_flags());
}


#endif /* __ASM__ECO32_IRQFLAGS_H */
