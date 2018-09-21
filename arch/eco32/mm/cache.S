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

#define FLUSH_ALL			7
#define FLUSH_DCACHE		3
#define INVALIDATE_ICACHE	4


	.global flush_cache_all
	.global flush_dcache
	.global flush_icache

	.text

flush_cache_all:
	cctl	FLUSH_ALL
	jr $31

flush_dcache:
	cctl	FLUSH_DCACHE
	jr $31

flush_icache:
	cctl	INVALIDATE_ICACHE
	jr $31
