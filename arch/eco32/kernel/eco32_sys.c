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


#include <linux/syscalls.h>


/*
 * on eco32 our PAGE_SHIFT is 12 so we could use mmap_pgoff as a
 * replacement for mmap2 but newer archs define it themself as
 * a forward to ksys_mmap_pgoff so we will do it the same way
 */
SYSCALL_DEFINE6(mmap2, unsigned long, addr, unsigned long, len,
                unsigned long, prot, unsigned long, flags,
                unsigned long, fd, off_t, offset)
{
    if (unlikely(offset & (~PAGE_MASK >> 12)))
        return -EINVAL;

    return ksys_mmap_pgoff(addr, len, prot, flags, fd,
                           offset >> (PAGE_SHIFT - 12));
}