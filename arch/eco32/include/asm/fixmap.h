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

#ifndef __ASM_ECO32_FIXMAP_H
#define __ASM_ECO32_FIXMAP_H


#include <linux/init.h>
#include <asm/page.h>
#include <asm/bug.h>
#include <asm/eco32.h>


/*
 * Here we define all the compile-time 'special' virtual
 * addresses. The point is to have a constant address at
 * compile time, but to set the physical address only
 * in the boot process. We allocate these special addresses
 * from the start of the consistent memory region upwards.
 * Also this lets us do fail-safe vmalloc(), we
 * can guarantee that these special addresses and
 * vmalloc()-ed addresses never overlap.
 *
 * these 'compile-time allocated' memory buffers are
 * fixed-size 4k pages. (or larger if used with an increment
 * higher than 1) use fixmap_set(idx,phys) to associate
 * physical memory with fixmap indices.
 */
enum fixed_addresses {
    __end_of_fixed_addresses
};


/*
 * on the eco32 we place this fixed pages at the start of the kernel
 * page mapped space. because this is the only region where we can put
 * user space protected page mapped virtual adresses
 */
#define FIXADDR_TOP         (ECO32_KERNEL_PAGE_MAPPED_START + (__end_of_fixed_addresses * PAGE_SIZE))
#define FIXADDR_SIZE        (__end_of_fixed_addresses << PAGE_SHIFT)
#define FIXADDR_START       (FIXADDR_TOP - FIXADDR_SIZE)

#define __fix_to_virt(x)    (FIXADDR_START + ((x) << PAGE_SHIFT))
#define __virt_to_fix(x)    (((x) - FIXADDR_START) >> PAGE_SHIFT)

#ifndef __ASSEMBLY__

void __init fixmap_init(void);


static __always_inline unsigned long fix_to_virt(const unsigned int idx)
{
    if (idx >= __end_of_fixed_addresses) {
        BUG();
    }

    return __fix_to_virt(idx);
}

static inline unsigned long virt_to_fix(const unsigned long vaddr)
{
    BUG_ON(vaddr >= FIXADDR_TOP || vaddr < FIXADDR_START);
    return __virt_to_fix(vaddr);
}

#endif

#endif
