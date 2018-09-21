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
 * eco32_timer.c -- the ECO32 timer clockevent and clocksource driver
 */


#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

#include <asm/io.h>


#define ECO32TIMER_DEV_NAME		"timerECO"
#define ECO32TIMER_CTL_IEN		0x2

/*
 * ECO32 Timer
 * 
 * An ECO32 Timer is running backwards at fixed 50Mhz
 * Writing to the divisor will load the counter and makes
 * the timer run. Once reached zero the expired bit will
 * bet set in the control register. reading the control
 * register will reset the expire bit
 * 
 * ctl : the control register
 * div : the devisor
 * cnt : the counter
 * 
 * This makes accessing the timer easier
 */
struct eco32_timer{
	unsigned int ctl;
	unsigned int div;
	unsigned int cnt;
};


/*
 * clocksource (continuously running timer)
 */

static struct eco32_timer* cs_timer;

static u64 eco32_clocksource_read(struct clocksource* cs)
{
	unsigned int count;

	/* use timer 0 */
	count = cs_timer->cnt;
	/* timer is counting backwards */
	return (u64) ~count;
}

static struct clocksource eco32_clocksource = {
	.name   = "eco32_clocksource",
	.rating = 300,
	.read   = eco32_clocksource_read,
	.mask   = CLOCKSOURCE_MASK(8 * sizeof(unsigned int)),
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init eco32_clocksource_init(struct device_node *node)
{	
	/* get base and remap timer */
	cs_timer = of_iomap(node, 0);
	
	if (!cs_timer) {
		pr_err("could not read and remap for clocksource timer\n");
		return;
	}
	
	/* set divisor and disable interrupts */
	cs_timer->div = 0xFFFFFFFF;
	cs_timer->ctl = 0x00;

	/* register clocksource */
	if (clocksource_register_hz(&eco32_clocksource, 50000000)) {
		pr_err("could not register clocksource\n");
	}
	
	pr_info("eco32 clocksource initialized\n");;
}



/*
 * clockevents (interrupting event timer)
 */

static struct eco32_timer* ce_timer;
static struct clock_event_device eco32_clockevent;

/*
 * The timer interrupt is mostly handled in generic code nowadays.
 *
 * This function just acknowledges the interrupt and fires the event
 * handler that has been set on the clockevent device by the generic
 * time management code.
 *
 * This function must be called by the timer exception handler, and
 * that's all the exception handler needs to do.
 */
static irqreturn_t eco32_clockevent_interrupt_handler(int irq, void* dev)
{
	unsigned int ctl;
	struct clock_event_device* evt;

	/* reset expire and irq enable */
	ctl = ce_timer->ctl;
	if (ctl) {
		ce_timer->ctl = 0;
	}

	evt = &eco32_clockevent;
	evt->event_handler(evt);

	return IRQ_HANDLED;
}

/*
 * Setup the timer for the next event
 * 
 * Delta is the timer ticks to the next interrupt.
 * This means we just load the divisor register with the delta
 * and enable interrupts again
 */
static int eco32_clockevent_set_event(unsigned long delta, struct clock_event_device* dev)
{
	/* start the timer */
	ce_timer->div = delta;
	ce_timer->ctl = ECO32TIMER_CTL_IEN;
	return 0;
}

static struct clock_event_device eco32_clockevent = {
	.name = "eco32_clockevent",
	.features = CLOCK_EVT_FEAT_ONESHOT,
	.rating = 300,
	.set_next_event = eco32_clockevent_set_event,
};

/*
 * Initialize the timer for the use of clockevents
 * 
 * This is the first to use if there are less timers
 * We need this one to get the system functional
 */
static int __init eco32_clockevents_init(struct device_node *node)
{
	unsigned int irq;
	
	/* get base and remap timer */
	ce_timer = of_iomap(node, 0);
	
	if (!ce_timer) {
		pr_err("could not read and remap clockevent timer\n");
		return 1;
	}
	
	irq = irq_of_parse_and_map(node, 0);
	if (!irq) {
		pr_err("could not read irq for clockevent timer\n");
		return 1;
	}
	
	/* reset irq enable */
	ce_timer->ctl = 0x00;

	if (request_irq(irq, eco32_clockevent_interrupt_handler, 0, ECO32TIMER_DEV_NAME, NULL)) {
		/*
		 * this one is brutal..
		 * the timer drives the scheduler without it the system will
		 * not be functional
		 */
		pr_err("could not request irq for clockevent timer\n");
		iounmap(ce_timer);
		return 1;
	}

	/* register clockevents */
	eco32_clockevent.cpumask = cpumask_of(0);
	clockevents_config_and_register(&eco32_clockevent, 50000000, 100, 0xFFFFFFFF);
	
	pr_info("eco32 clockevent initialized\n");
	
	return 0;
}


/*
 * Initialize the timers
 */
static int __init eco32_timer_init(struct device_node *node)
{
	static int clockevents_initialized = 0;
	
	/*
	 * We really need that clockevents timer running
	 * 
	 * If it fails on the first try, try again with the next timer
	 * If that fails again we will have a problem..
	 */
	if (!clockevents_initialized) {
		clockevents_initialized = !eco32_clockevents_init(node);
	}else{
		eco32_clocksource_init(node);
	}
	
	return 0;
}
TIMER_OF_DECLARE(eco32_timer, "thm,eco32-timer", eco32_timer_init);
