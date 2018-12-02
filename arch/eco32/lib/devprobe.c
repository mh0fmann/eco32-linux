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


#include <asm/irq.h>
#include <asm/devprobe.h>
#include <asm/io.h>
#include <linux/spinlock.h>


static int bus_timeout;
static void ISR_bus_timeout(int irq, struct pt_regs* regs)
{
	bus_timeout = 1;
	regs->xa += 4;
}


/*
 * Probe if the given address is present on the bus
 * 
 * This is used by all eco32 drivers to check if the
 * deive is present on the bus.
 * 
 * NOTE: this function turns off irqs while it is running.
 * this is neccessary to savely change ISR for BUS_TIMEOUT_EXCEPTION
 */
extern int eco32_device_probe(unsigned long address){
	
	int ret = 0;
	spinlock_t lock;
	unsigned long flags;
	isr_t timeout_isr;
	
	bus_timeout = 0;
	
	spin_lock_init(&lock);
	spin_lock_irqsave(&lock, flags);
	timeout_isr = get_ISR(XCPT_BUS_TIMEOUT);
	set_ISR(XCPT_BUS_TIMEOUT, ISR_bus_timeout);
	
	ioread32be((void*)address);
	
	if(bus_timeout){
		ret = -1;
	}
	
	set_ISR(XCPT_BUS_TIMEOUT, timeout_isr);
	spin_unlock_irqrestore(&lock, flags);
	
	return ret;
}
