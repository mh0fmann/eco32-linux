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

    .global memset_io


/*
 * on eco32 all iomem has to be accessed thorugh 32bit read and write
 */


memset_io:
    addi    $8,$5,0
    andi    $8,$8,0xFF
    slli    $5,$5,8
    or      $5,$5,$8
    slli    $5,$5,8
    or      $5,$5,$8
    slli    $5,$5,8
    or      $5,$5,$8
    addi    $8,$0,0
loop:
    stw     $5,$4,0
    addi    $4,$4,4
    addi    $8,$8,4
    blt     $8,$6,loop
    jr      $31
