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
 * archlocalirq.S -- IRQ support functions for interrupt save and restore
 */

#define ARCH_IRQ_SHIFT		23

/*
 * unsigned long arch_local_save_flags(void);
 */
	.text
	.global	arch_local_save_flags
arch_local_save_flags:
	mvfs	$2,0
	slri	$2,$2,ARCH_IRQ_SHIFT
	andi	$2,$2,1
	jr	$31

/*
 * void arch_local_irq_restore(unsigned long flags);
 */
	.text
	.global	arch_local_irq_restore
arch_local_irq_restore:
	bne	$4,$0,arch_local_irq_set
arch_local_irq_clear:
	addi	$4,$0,1
	slli	$4,$4,ARCH_IRQ_SHIFT
	xnori	$4,$4,0
	mvfs	$2,0
	and	$2,$2,$4
	mvts	$2,0
	jr	$31
arch_local_irq_set:
	addi	$4,$0,1
	slli	$4,$4,ARCH_IRQ_SHIFT
	mvfs	$2,0
	or	$2,$2,$4
	mvts	$2,0
	jr	$31
