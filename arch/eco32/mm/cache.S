/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cacheflush.S -- function decleration for flushing caches
 */

#define FLUSH_ALL			7
#define FLUSH_DCACHE		3
#define INVALIDATE_ICACHE	4

/*
 * void flush_cache_all(void);
 */
	.text
	.global flush_cache_all
flush_cache_all:
	cctl	FLUSH_ALL
	jr $31

/*
 * void flush_dcache(void);
 */
	.text
	.global flush_dcache
flush_dcache:
	cctl	FLUSH_DCACHE
	jr $31

/*
 * void flush_icache(void);
 */
	.text
	.global flush_icache
flush_icache:
	cctl	INVALIDATE_ICACHE
	jr $31
