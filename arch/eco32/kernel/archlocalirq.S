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

#define ARCH_IRQ_SHIFT      23

    .global arch_local_save_flags
    .global arch_local_irq_restore

    .text


arch_local_save_flags:
    mvfs    $2,0
    slri    $2,$2,ARCH_IRQ_SHIFT
    andi    $2,$2,1
    jr      $31


arch_local_irq_restore:
    addi    $5,$0,1
    slli    $5,$5,ARCH_IRQ_SHIFT
    bne     $4,$0,arch_local_irq_set
    xnori   $5,$5,0
    mvfs    $2,0
    and     $2,$2,$5
    mvts    $2,0
    jr      $31
arch_local_irq_set:
    mvfs    $2,0
    or      $2,$2,$5
    mvts    $2,0
    jr      $31
