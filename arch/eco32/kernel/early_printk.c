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


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/console.h>


/*
 * The ECO32 early printk uses the first uart line
 * to push out the messages.
 * 
 * this is a rather simple polling prinkt. but it is enough
 * to get the chars out during early boot
 * 
 * once we sat up the serial line driver we switch to that
 * anyways
 */

static void eco32_early_putc(char c)
{
    volatile unsigned int* base;

    base = (unsigned int*) 0xF0300000;

    if (c == '\n') {
        eco32_early_putc('\r');
    }

    while ((*(base + 2) & 1) == 0) ;

    *(base + 3) = c;
}


static void eco32_early_puts(struct console* con,
                             const char* s,
                             unsigned n)
{
    while (n-- && *s) {
        eco32_early_putc(*s++);
    }
}


static struct console eco32_earlycon = {
    .name  = "eco32_earlycon",
    .write = eco32_early_puts,
    .flags = CON_PRINTBUFFER | CON_BOOT,
    .index = -1,
};


void __init setup_early_printk(void)
{
    if (early_console != NULL) {
        return;
    }

    early_console = &eco32_earlycon;
    register_console(early_console);
}
