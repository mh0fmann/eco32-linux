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


/*
 * musl is our only libc we currently have
 * and it does not support renameat2 so we need the old one
 */
#define __ARCH_WANT_RENAMEAT

/*
 * since v4.20 archs that do not use the newer statx interface
 * need to set this to get the syscalls
 */
#define __ARCH_WANT_STAT64

/*
 * we do not call these directly so we need the wrappers to
 * be present
 */
#define __ARCH_WANT_SYS_FORK
#define __ARCH_WANT_SYS_CLONE

#include <asm-generic/unistd.h>
