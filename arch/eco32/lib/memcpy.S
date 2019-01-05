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

#include <linux/linkage.h>


/*
 * on eco32 all iomem has to be accessed through 32bit read and write
 * also all iomem is algigned to 32bit boundries so we don't need to
 * be aware of unalined reads and writes
 */

ENTRY(memcpy_tofromio)
    addi    $8,$0,0
loop:
    ldw     $9,$5,0
    stw     $9,$4,0
    addi    $4,$4,4
    addi    $5,$5,4
    addi    $8,$8,4
    ble     $8,$6,loop
    jr      $31
END(memcpy_tofromio)
