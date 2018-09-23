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



#include <linux/seq_file.h>
#include <linux/delay.h>


static void* c_start(struct seq_file* m, loff_t* pos)
{
    //we only have one CPU
    return *pos < 1 ? (void*) 1 : NULL;
}


static void* c_next(struct seq_file* m, void* v, loff_t* pos)
{
    ++*pos;
    return NULL;
}


static void c_stop(struct seq_file* m, void* v)
{
}


static int show_cpuinfo(struct seq_file* m, void* v)
{
    seq_printf(m,
               "cpu\t\t: ECO32\n"
               "frequency\t: %ld\n"
               "bogomips\t: %lu.%02lu\n",
               loops_per_jiffy * HZ,
               (loops_per_jiffy * HZ) / 500000,
               ((loops_per_jiffy * HZ) / 5000) % 100);
    return 0;
}


const struct seq_operations cpuinfo_op = {
    .start = c_start,
    .next = c_next,
    .stop = c_stop,
    .show = show_cpuinfo,
};
