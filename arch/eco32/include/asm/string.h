/*
 * Linux architectural port borrowing liberally from similar works of
 * others, namely OpenRISC and RISC-V.  All original copyrights apply
 * as per the original source declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2019 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#ifndef __ASM_ECO32_STRING_H
#define __ASM_ECO32_STRING_H

#define __HAVE_ARCH_MEMSET
extern void* memset(void*, int, size_t);

#endif /* __ASM_ECO32_STRING_H */