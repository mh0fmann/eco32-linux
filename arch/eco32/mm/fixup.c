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

#include <linux/extable.h>
#include <asm/uaccess.h>


int fixup_exception(struct pt_regs* regs)
{
    const struct exception_table_entry* entry;
    entry = search_exception_tables(regs->xa);

    if (entry) {
        pr_debug("fixup 0x%08lx found for insn at 0x%08lx\n",
                 entry->fixup, regs->xa);
        regs->xa = entry->fixup;
        return 1;
    }

    pr_debug("no fixup found for insn at 0x%08lx\n",
             regs->xa);
    return 0;
}
