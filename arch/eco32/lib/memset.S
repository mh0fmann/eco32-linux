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
 * hand optimized memset routine which uses word size memset
 * if possible to speed up the memeset
 */
ENTRY(memset)
    /* return src pointer */
    ori     $2,$4,0

    /* exit if n == 0 */
    beq     $6,$0,memset_done

    /* truncat c to char */
    andi    $8,$5,0xFF

    /* Check for at least two whole words */
    addi    $9,$6,0     # set $9 = n
    addi    $11,$4,0    # set $11 = src
    addi    $10,$0,8
    blt     $6,$10,set_remaining_bytes

    /* mask out two LSBs to check aligment */
    andi    $10,$4,0x3

    /* lsb == 00?, jump to word copy loop */
    beq     $10,$0,set_words

set_leading_bytes:
    /* lsb == 01,10 or 11 */
    stb     $8,$4,0     # src = [ccc]c
    subi    $9,$9,1     # Decrease n
    addi    $11,$11,1   # src += 1

    /* lsb == 11? */
    addi    $12,$0,0x3
    beq     $10,$12,set_words

    /* lsb == 01 or 10 */
    stb     $8,$4,1     # src+1 = [ccc]c
    subi    $9,$9,1     # Decrease n
    addi    $11,$11,1   # src += 1

    /* lsb == 10? */
    addi    $12,$0,0x2
    beq     $10,$12,set_words

    /* lsb == 01 */
    stb     $8,$4,2     # src+2 = [ccc]c
    subi    $9,$9,1     # Decrease n
    addi    $11,$11,1   # src += 1

    /* word set loop */
set_words:
    /* skip word extension if c is 0 */
    beq     $8,$0,skip_word_extension

    /* extend char c to 32 bit word cccc in $8 */
    slli    $12,$8,16   # $8 = 000c, $9 = 0c00
    or      $8,$8,$12   # $8 = 0c0c, $9 = 0c00
    slli    $12,$8,8    # $8 = 0c0c, $9 = c0c0
    or      $8,$8,$12

skip_word_extension:
    addi    $10,$0,4
word_set_loop:
    stw     $8,$11,0    # src = cccc
    subi    $9,$9,4     # Decrease n
    addi    $11,$11,4   # Increase src
    bge     $9,$10,word_set_loop
    beq     $9,$0,memset_done

    /* byte set loop */
set_remaining_bytes:
    stb     $8,$11,0    # src = [ccc]c
    subi    $9,$9,1     # Decrease n
    addi    $11,$11,1   # Increase src
    bne     $9,$0,set_remaining_bytes
memset_done:
    jr      $31
END(memset)

/*
 * on eco32 all iomem has to be accessed through 32bit read and write
 * also all iomem is algigned to 32bit boundries so we don't need to
 * be aware of unalined reads and writes
 */

ENTRY(memset_io)
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
END(memset_io)
