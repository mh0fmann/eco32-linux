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
 * irq.h -- Interrupt handling
 */

#ifndef __ASM_ECO32_IRQ_H
#define __ASM_ECO32_IRQ_H

#include <asm/ptrace.h>

/* NR_IRQS counts only real interrupts, but not faults */
#define NR_IRQS			16

#define NO_IRQ			(-1)

#define XCPT_BUS_TIMEOUT	16
#define XCPT_ILL_INST		17
#define XCPT_PRV_INST		18
#define XCPT_DIV_INST		19
#define XCPT_TRAP_INST		20
#define XCPT_TLB_MISS		21
#define XCPT_TLB_WRITE		22
#define XCPT_TLB_INVAL		23
#define XCPT_ILL_ADDR		24
#define XCPT_PRV_ADDR		25

#define irq_canonicalize(irq)	(irq)

#define interrupts_enabled(regs) ((regs->psw >> 22) & 0x1)

typedef void (*isr_t)(int irq, struct pt_regs* regs);

void set_ISR(int irq, isr_t isr);
isr_t get_ISR(int irq);

void init_IRQ(void);
void do_IRQ(int irq, struct pt_regs* regs);

extern void def_xcpt_handler(int irq, struct pt_regs* regs);

static inline void or_irq_mask(unsigned long mask)
{
	__asm__ ("andi $8,%0,0x0000FFFF	\n"
	         "mvfs $9,0				\n"
	         "or $9,$9,$8			\n"
	         "mvts $9,0				\n"
	         : : "r"(mask) : "$8", "$9");
}

static inline void and_irq_mask(unsigned long mask)
{
	__asm__ ("ori $8,%0,0x0000FFFF	\n"
	         "mvfs $9,0				\n"
	         "and $9,$9,$8			\n"
	         "mvts $9,0				\n"
	         : : "r"(mask) : "$8", "$9");
}

#endif /* __ASM_ECO32_IRQ_H */
