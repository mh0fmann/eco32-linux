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

#ifndef __ASM_ECO32_H
#define __ASM_ECO32_H

/*
 * Makros for the ECO32 virtual memory layout
 * used at some of the memory managment
 * related makros and code
 */
#define ECO32_USER_PAGE_MAPPED_START            0x00000000
#define ECO32_USER_PAGE_MAPPED_SIZE             0x80000000
#define ECO32_USER_PAGE_MAPPED_END              ECO32_USER_PAGE_MAPPED_START + ECO32_USER_PAGE_MAPPED_SIZE
#define ECO32_KERNEL_PAGE_MAPPED_START          0x80000000
#define ECO32_KERNEL_PAGE_MAPPED_SIZE           0x40000000
#define ECO32_KERNEL_PAGE_MAPPED_END            ECO32_KERNEL_PAGE_MAPPED_START + ECO32_KERNEL_PAGE_MAPPED_SIZE
#define ECO32_KERNEL_DIRECT_MAPPED_RAM_START    0xC0000000
#define ECO32_KERNEL_DIRECT_MAPPED_RAM_SIZE     0x20000000
#define ECO32_KERNEL_DIRECT_MAPPED_RAM_END      ECO32_KERNEL_DIRECT_MAPPED_RAM_START + ECO32_KERNEL_DIRECT_MAPPED_RAM_SIZE
#define ECO32_KERNEL_DIRECT_MAPPED_ROM_START    0xE0000000
#define ECO32_KERNEL_DIRECT_MAPPED_ROM_SIZE     0x10000000
#define ECO32_KERNEL_DIRECT_MAPPED_ROM_END      ECO32_KERNEL_DIRECT_MAPPED_ROM_START + ECO32_KERNEL_DIRECT_MAPPED_ROM_SIZE
#define ECO32_KERNEL_DIRECT_MAPPED_IO_START     0xF0000000
#define ECO32_KERNEL_DIRECT_MAPPED_IO_SIZE      0x10000000
#define ECO32_KERNEL_DIRECT_MAPPED_IO_END       ECO32_KERNEL_DIRECT_MAPPED_IO_START + ECO32_KERNEL_DIRECT_MAPPED_IO_SIZE

#endif
