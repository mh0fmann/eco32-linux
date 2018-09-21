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

#include <asm-generic/vmlinux.lds.h>
#include <asm/page.h>
#include <asm/cache.h>
#include <asm/thread_info.h>

OUTPUT_ARCH(eco32)
ENTRY(_start)			/* in head.S */

jiffies = jiffies_64 + 4;	/* big-endian lower 32 bits */

SECTIONS
{
	. = 0xC0000000;

	.head.text : {
		_text = .;
		HEAD_TEXT
	}

	.text : {
		_stext = .;
		TEXT_TEXT
		SCHED_TEXT
		CPUIDLE_TEXT
		LOCK_TEXT
		IRQENTRY_TEXT
		_etext = .;
	}

	EXCEPTION_TABLE(16)

	_sdata = .;
	RO_DATA_SECTION(PAGE_SIZE)
	RW_DATA_SECTION(L1_CACHE_BYTES, PAGE_SIZE, THREAD_SIZE)
	_edata = .;

	. = ALIGN(PAGE_SIZE);
	.init.begin : {
		__init_begin = .;
	}
	INIT_TEXT_SECTION(PAGE_SIZE)
	INIT_DATA_SECTION(16)
	.exit.text : {
		EXIT_TEXT
	}
	.exit.data : {
		EXIT_DATA
	}
	. = ALIGN(PAGE_SIZE);
	.init.end : {
		__init_end = .;
	}

	BSS_SECTION(0, 0, 0)
	_end = .;

	STABS_DEBUG
	DWARF_DEBUG
	NOTES
	DISCARDS
}
