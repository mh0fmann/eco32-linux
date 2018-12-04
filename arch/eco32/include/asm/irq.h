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

#ifndef __ASM_ECO32_IRQ_H
#define __ASM_ECO32_IRQ_H

#include <asm/ptrace.h>

/* NR_IRQS counts only real interrupts, but not faults */
#define NR_IRQS                 16
/* NR_INTERRUPS counts all interrupts including faults */
#define NR_INTERRUPTS           32

#define NO_IRQ                  (-1)

#define XCPT_BUS_TIMEOUT        16
#define XCPT_ILL_INST           17
#define XCPT_PRV_INST           18
#define XCPT_DIV_INST           19
#define XCPT_TRAP_INST          20
#define XCPT_TLB_MISS           21
#define XCPT_TLB_WRITE          22
#define XCPT_TLB_INVAL          23
#define XCPT_ILL_ADDR           24
#define XCPT_PRV_ADDR           25

#define IRQ_TERMINAL1_TX        0
#define IRQ_TERMINAL1_RX        1
#define IRQ_TERMINAL2_TX        2
#define IRQ_TERMINAL2_RX        3
#define IRQ_KEYBOARD            4
#define IRQ_DISK                8
#define IRQ_TIMER1              14
#define IRQ_TIMER2              15


#ifndef __ASSEMBLY__

#define irq_canonicalize(irq)   (irq)

#define interrupts_enabled(regs) ((regs->psw >> 22) & 0x1)


/*
 * Signature for all ISR-functions that get called at the very end
 * of our kernel-entry
 */
typedef void (*isr_t)(int irq, struct pt_regs* regs);


/*
 * The isr table that hold the isr addresses
 */
extern isr_t isr_tbl[NR_INTERRUPTS];


/*
 * Write a new isr for a given irq in the isr table
 */
static inline int set_ISR(unsigned int irq, isr_t isr)
{
    if (irq >= NR_INTERRUPTS) {
        return 1;
    }

    isr_tbl[irq] = isr;

    return 0;
}

/*
 * Read a isr for a given irq from the isr table
 */
static inline isr_t get_ISR(unsigned int irq)
{
    if (irq >= NR_INTERRUPTS) {
        /* NULL is not present here */
        return (isr_t)(0);
    }

    return isr_tbl[irq];
}

/*
 * default ISR that generates a panic when called
 */
extern void def_xcpt_handler(int irq, struct pt_regs* regs);


/* initialize the arch specific interrupt stuff for the irq subsystem */
void init_IRQ(void);


/*
 * do_IRQ handles all normal device interrupts
 * This is our ISR for all device interrupts.
 * The devices itslef register thier interrupt handler
 * via request_irq which get called by generic_handle_irq
 */
void do_IRQ(int irq, struct pt_regs* regs);

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ECO32_IRQ_H */
