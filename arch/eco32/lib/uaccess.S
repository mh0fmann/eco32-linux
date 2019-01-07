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
#include <asm/errno.h>


/*
 * unsigned long __copy_tofrom_user(void *to,
 *                                  const void *from,
 *                                  unsigned long size);
 *
 * NOTE: Returns number of bytes NOT copied!
 */
ENTRY(__copy_tofrom_user)
    beq     $6,$0,copy_done     # nothing to do?
    add     $2,$6,$0            # return what's left
copy_get:
    ldbu    $8,$5,0             # get byte
copy_put:
    stb     $8,$4,0             # put byte
    addi    $4,$4,1             # bump ptrs
    addi    $5,$5,1
    subi    $2,$2,1             # decrement
    bne     $2,$0,copy_get      # done?
copy_done:
    jr      $31
ENDPROC(__copy_tofrom_user)

    .section    .fixup,"ax"
copy_fixup:
    j       copy_done
    .previous

    .section    __ex_table,"a"
    .long   copy_get,copy_fixup
    .long   copy_put,copy_fixup
    .previous


/*
 * unsigned long __clear_user(void *addr,
 *                            unsigned long size);
 *
 * NOTE: Returns number of bytes NOT cleared!
 */
ENTRY(__clear_user)
    beq     $5,$0,clear_done    # nothing to do?
    add     $2,$5,$0            # return what's left
clear_put:
    stb     $0,$4,0             # put zero byte
    addi    $4,$4,1             # bump ptr
    subi    $2,$2,1             # decrement
    bne     $2,$0,clear_put     # done?
clear_done:
    jr  $31
ENDPROC(__clear_user)

    .section    .fixup,"ax"
clear_fixup:
    j       clear_done
    .previous

    .section    __ex_table,"a"
    .long   clear_put,clear_fixup
    .previous
