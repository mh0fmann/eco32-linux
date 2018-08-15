/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2017 Hellwig Geisse
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * topology.c -- ??
 */

#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/init.h>
#include <linux/node.h>
#include <linux/nodemask.h>
#include <linux/percpu.h>

static DEFINE_PER_CPU(struct cpu, cpu_devices);

static int __init topology_init(void)
{
	int i, ret;

#ifdef CONFIG_NUMA
	for_each_online_node(i)
	register_one_node(i);
#endif /* CONFIG_NUMA */

	for_each_present_cpu(i) {
		struct cpu* c = &per_cpu(cpu_devices, i);

		c->hotpluggable = 1;
		ret = register_cpu(c, i);

		if (ret)
			pr_warn("topology_init: register_cpu %d "
			        "failed (%d)\n", i, ret);
	}

	return 0;
}

subsys_initcall(topology_init);
