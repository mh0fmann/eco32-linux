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

#define __ARCH_HAVE_MMU

#define sys_mmap2   sys_mmap_pgoff

#define __ARCH_WANT_RENAMEAT
#define __ARCH_WANT_SYS_FORK
#define __ARCH_WANT_SYS_CLONE

#include <asm-generic/unistd.h>
