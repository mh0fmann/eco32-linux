/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2017 Hellwig Geisse
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * string.S -- low-level system/user memory copy and clear routines
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
	.global	__copy_tofrom_user
__copy_tofrom_user:
	subi	$29,$29,8
	stw	$4,$29,0		# save dst ptr
	stw	$5,$29,4		# save src ptr
	add	$2,$6,$0		# no need to save size
copy_next:
	beq	$2,$0,copy_done
	subi	$2,$2,1			# decrement count
copy_get:
	ldbu	$8,$5,0			# get byte
copy_put:
	stb	$8,$4,0			# put byte
	addi	$4,$4,1			# bump ptrs
	addi	$5,$5,1
	j	copy_next
copy_done:
	ldw	$4,$29,0		# restore dst ptr
	ldw	$5,$29,4		# restore src ptr
	addi	$29,$29,8
	jr	$31

	.section	.fixup,"ax"
copy_fixup:
	j	copy_done
	.previous

	.section	__ex_table,"a"
	.long	copy_get,copy_fixup
	.long	copy_put,copy_fixup
	.previous


/*
 * unsigned long __clear_user(void *addr,
 *                            unsigned long size);
 *
 * NOTE: Returns number of bytes NOT cleared!
 */
	.global	__clear_user
__clear_user:
	subi	$29,$29,4
	stw	$4,$29,0		# save dst ptr
	add     $2,$5,$0		# no need to save size
clear_next:
	beq	$2,$0,clear_done
	subi	$2,$2,1			# decrement count
clear_put:
	stb	$0,$4,0			# put zero byte
	addi	$4,$4,1			# bump ptr
	j	clear_next
clear_done:
	ldw	$4,$29,0		# restore dst ptr
	addi	$29,$29,4
	jr	$31

	.section	.fixup,"ax"
clear_fixup:
	j	clear_done
	.previous

	.section	__ex_table,"a"
	.long	clear_put,clear_fixup
	.previous
