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
 * irq.c -- Interrupt initialization and handling entry
 */

#include <linux/interrupt.h>
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/sched.h>

#include <asm/siginfo.h>
#include <asm/signal.h>
#include <asm/irq.h>


/*
 * We safe the current state of active irqs here so we can
 * load the active irqs from here.
 *
 * This needs be done so we can update the psw on each context
 * switch and return so all threads run with the correct irqs
 * masked
 */
unsigned long volatile irqmask = 0;


static void enable_eco32_irq(struct irq_data* data)
{
	irqmask |= (1 << data->irq);
	or_irq_mask(1 << data->irq);
}


static void disable_eco32_irq(struct irq_data* data)
{
	irqmask &= ~(1 << data->irq);
	and_irq_mask(~(1 << data->irq));
}


static struct irq_chip eco32_irq_type = {
	.name = "ECO32",
	.irq_disable = disable_eco32_irq,
	.irq_mask = disable_eco32_irq,
	.irq_unmask = enable_eco32_irq,
};


/*
 * initialise the interrupt system
 */
void __init init_IRQ(void)
{
	int irq;

	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_set_chip_and_handler(irq,
		                         &eco32_irq_type,
		                         handle_level_irq);
	}
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
	generic_handle_irq(irq);
	irq_exit();

	/* restore original pointer to exception frame */
	set_irq_regs(old_regs);
}
