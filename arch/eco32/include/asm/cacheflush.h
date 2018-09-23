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

#ifndef __ASM_ECO32_CACHEFLUSH_H
#define __ASM_ECO32_CACHEFLUSH_H

#define flush_cache_all()   __asm__("cctl 7")
#define flush_dcache()      __asm__("cctl 3")
#define flush_icache()      __asm__("cctl 4")


/*
 * The ECO32 can only flush either the complete instruction
 * or the complete data cache
 * 
 * flushing specific parts of it is not supported
 */
#define flush_cache_mm(mm)                              flush_cache_all()
#define flush_cache_dup_mm(mm)                          flush_cache_all()
#define flush_cache_range(vma, addr, pfn)               flush_cache_all()
#define flush_cache_page(vma, addr, pfn)                flush_cache_all()

#define ARCH_IMPLEMENTS_FLUSH_DCACHE_PAGE 0
#define flush_dcache_page(page)                         flush_dcache()
#define flush_dcache_mmap_lock(mapping)                 flush_dcache()
#define flush_dcache_mmap_unlock(mapping)               flush_dcache()

#define flush_icache_range(start, end)                  flush_icache()
#define flush_icache_page(vma, page)                    flush_icache()
#define flush_icache_user_range(vma, page, adr, len)    flush_icache()

#define flush_cache_vmap(start, end)                    flush_cache_all()
#define flush_cache_vunmap(start, end)                  flush_cache_all()


#define copy_to_user_page(vma, page, vaddr, dst, src, len) \
    do { \
        memcpy(dst, src, len); \
        flush_icache_user_range(vma, page, vaddr, len); \
    } while (0)
#define copy_from_user_page(vma, page, vaddr, dst, src, len) \
    memcpy(dst, src, len)

#endif /* __ASM_ECO32_CACHEFLUSH_H */
