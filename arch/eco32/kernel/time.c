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
 * time.c -- the ECO32 timer driver
 */


#include <linux/kernel.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/ftrace.h>

#include <asm/irq.h>


#define ECO32TIMER_DEV_NAME		"timerECO"


#define ECO32TIMER0_BASE		0xF0000000
#define ECO32TIMER0_CTL			(*(unsigned int*)ECO32TIMER0_BASE)
#define ECO32TIMER0_DIV			(*(unsigned int*)(ECO32TIMER0_BASE+4))

#define ECO32TIMER1_IRQ_NR		15
#define ECO32TIMER1_BASE		0xF0001000
#define ECO32TIMER1_CTL			(*(unsigned int*)ECO32TIMER1_BASE)
#define ECO32TIMER1_DIV			(*(unsigned int*)(ECO32TIMER1_BASE+4))

#define ECO32TIMER_CTL_IEN		0x2

#define ECO32TIMERX_CTL_READ(TIMERBASE)	__asm__("ldw $0,%0,0;"::"r"(TIMERBASE))


/**************************************************************/

/*
 * clocksource (continuously running timer)
 *
 * based on ECO32 timer 0
 * 32 bits, running at 50 MHz
 */


static u64 eco32_clocksource_read(struct clocksource* cs)
{
	unsigned int count;

	/* use timer 0 */
	count = *((unsigned int*)ECO32TIMER0_BASE + 2);
	/* timer is counting backwards */
	return (u64) ~count;
}


static struct clocksource eco32_clocksource = {
	.name   = "eco32_clocksource",
	.rating = 300,
	.read   = eco32_clocksource_read,
	.mask   = CLOCKSOURCE_MASK(32),
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS,
};


static void __init eco32_clocksource_init(void)
{
	/* use timer 0, set divisor = 0xFFFFFFFF, reset irq enable */
	ECO32TIMER0_DIV = 0xFFFFFFFF;
	ECO32TIMER0_CTL = 0x00;
	ECO32TIMERX_CTL_READ(ECO32TIMER0_BASE);

	/* register clocksource */
	if (clocksource_register_hz(&eco32_clocksource, 50000000)) {
		panic("failed to register clocksource");
	}
}


/**************************************************************/

/*
 * clockevents (interrupting event timer)
 *
 * based on ECO32 timer 1
 * 32 bits, running at 50 MHz
 */


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
static irqreturn_t eco32timer1_interrupt_handler(int irq, void* dev)
{

	struct clock_event_device* evt;

	/* use timer 1, reset expired bit, reset irq enable */
	ECO32TIMER1_CTL = 0x00;
	ECO32TIMERX_CTL_READ(ECO32TIMER1_BASE);

	evt = &eco32_clockevent;
	evt->event_handler(evt);

	return IRQ_HANDLED;
}


static int eco32_clockevent_set_event(unsigned long delta, struct clock_event_device* dev)
{
	ECO32TIMER1_DIV = delta;
	ECO32TIMER1_CTL = ECO32TIMER_CTL_IEN;
	return 0;
}


static struct clock_event_device eco32_clockevent = {
	.name = "eco32_clockevent",
	.features = CLOCK_EVT_FEAT_ONESHOT,
	.rating = 300,
	.set_next_event = eco32_clockevent_set_event,
};


static void __init eco32_clockevents_init(void)
{
	/* use timer 1, reset irq enable */
	ECO32TIMER1_CTL = 0x00;
	ECO32TIMERX_CTL_READ(ECO32TIMER1_BASE);

	/* set timer ISR */
	set_ISR(ECO32TIMER1_IRQ_NR, do_IRQ);

	if (request_irq(ECO32TIMER1_IRQ_NR, eco32timer1_interrupt_handler, 0, ECO32TIMER_DEV_NAME, NULL)) {
		/*
		 * this one is brutal..
		 * the timer drives the scheduler without it the system will
		 * not be functional
		 */
		panic("could not request irq for eco32timer\n");
	}

	/* register clockevents */
	eco32_clockevent.cpumask = cpumask_of(0);
	clockevents_config_and_register(&eco32_clockevent, 50000000, 100, 0xFFFFFFFF);
}


/**************************************************************/

/* initialization */


void __init time_init(void)
{
	eco32_clocksource_init();
	eco32_clockevents_init();
}
