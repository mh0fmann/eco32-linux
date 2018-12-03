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

#include <linux/interrupt.h>
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/irqdomain.h>

#include <asm/irq.h>


/*
 * initialise the interrupt system
 */
void __init init_IRQ(void)
{
    /*
     * Set do_IRQ as the ISR for all currently used irqlines
     * used by hardware on the SoC
     *
     * Other irqlines should never be active and interrupt the cpu.
     * If they do we got some hardwarefailure which gets
     * catched by the default ISR that is still present for these
     * entries
     */
    set_ISR(IRQ_TERMINAL1_TX, do_IRQ);
    set_ISR(IRQ_TERMINAL1_RX, do_IRQ);
    set_ISR(IRQ_TERMINAL2_TX, do_IRQ);
    set_ISR(IRQ_TERMINAL2_RX, do_IRQ);
    set_ISR(IRQ_KEYBOARD, do_IRQ);
    set_ISR(IRQ_DISK, do_IRQ);
    set_ISR(IRQ_TIMER1, do_IRQ);
    set_ISR(IRQ_TIMER2, do_IRQ);

    /*
     * Initialize the irq chip to get the interrupt subsystem
     * up and running
     */
    irqchip_init();
}


/*
 * do_IRQ handles all normal device interrupts
 * This is our ISR for all device interrupts.
 * The devices itslef register thier interrupt handler
 * via request_irq which get called by generic_handle_irq
 */
void __irq_entry do_IRQ(int irq, struct pt_regs* regs)
{
    /* establish pointer to current exception frame */
    struct pt_regs* old_regs = set_irq_regs(regs);

    /*
     * notify kernel over hardware interrupt and let kernel call the
     * registered handler
     */
    irq_enter();
    generic_handle_irq(irq_find_mapping(NULL, irq));
    irq_exit();

    /* restore original pointer to exception frame */
    set_irq_regs(old_regs);
}
