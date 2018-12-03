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


#include <asm/eco32.h>

    .global eco32_restart

    .text


/*
 * Restarting is fairly easy atm.
 * We just disable interrupts and jump back to the rom
 */
eco32_restart:
    mvts    $0,0
    ori     $4,$0,ECO32_KERNEL_DIRECT_MAPPED_ROM_START
    jalr    $4

