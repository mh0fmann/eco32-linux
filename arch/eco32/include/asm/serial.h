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
 * serial.h -- defines baud rate of serial line
 */

#ifndef __ASM_ECO32_SERIAL_H
#define __ASM_ECO32_SERIAL_H

#ifdef __KERNEL__

#include <asm/cpuinfo.h>

/* There's a generic version of this file, but it assumes a 1.8MHz UART clk...
 * this, on the other hand, assumes the UART clock is tied to the system
 * clock... 8250_early.c (early 8250 serial console) actually uses this, so
 * it needs to be correct to get the early console working.
 */

#define BASE_BAUD (cpuinfo.clock_frequency/16)

#endif /* __KERNEL__ */

#endif /* __ASM_ECO32_SERIAL_H */
