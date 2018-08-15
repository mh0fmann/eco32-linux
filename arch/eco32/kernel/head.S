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
 * head.S -- Linux kernel assembler startup
 */

/* these includes are needed for THREAD_SIZE */
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/thread_info.h>

#define PSW		0
#define TLB_INDEX	1
#define TLB_ENTRY_HI	2
#define TLB_ENTRY_LO	3
#define TLB_ENTRIES	32

	.global	_start

	.global	xcept
	.global	umiss
	.global	current_ti
	.global	eco32_early_setup

	.section	".head.text","ax"

	#------------------------------------

	# the bootloader jumps here
	.org	0x00000000
_start:
	j	start

	# exceptions (interrupts and faults) arrive here
	.org	0x00000004
	j	xcept

	# TLB misses with addresses < 0x80000000 arrive here
	.org	0x00000008
	j	umiss

	#------------------------------------

start:
	# init PSW
	# disable interrupts
	# interrupt vector points to RAM
	ldhi	$8,0x08000000
	mvts	$8,PSW

	# invalidate TLB
	# fill with direct-mapped virtual addresses
	mvts	$0,TLB_ENTRY_LO
	ldhi	$8,0xC0000000
	mvts	$8,TLB_ENTRY_HI
	addi	$8,$0,0
	addi	$9,$0,TLB_ENTRIES
tlb_inv_loop:
	mvts	$8,TLB_INDEX
	tbwi
	addi	$8,$8,1
	bne	$8,$9,tlb_inv_loop

	# setup thread and stack pointers
	ldw	$27,$0,current_ti
	addi	$29,$27,THREAD_SIZE

	# clear BSS
	addi	$8,$0,__bss_start
	addi	$9,$0,_end
	j	bss_clr_test
bss_clr_loop:
	stw	$0,$8,0
	addi	$8,$8,4
bss_clr_test:
	bltu	$8,$9,bss_clr_loop

	# initialize device tree handling
	# $4 must hold either a pointer to a valid FDT
	# or NULL (then the built-in FDT is used)
	addi	$4,$0,0
	jal	eco32_early_setup

	# jump to the start_kernel() routine in init/main.c
	j	start_kernel


	.section .data,"aw"
	.align	4096
	.global empty_zero_page
empty_zero_page:
	.space	4096

	.global swapper_pg_dir
swapper_pg_dir:
	.space	4096
