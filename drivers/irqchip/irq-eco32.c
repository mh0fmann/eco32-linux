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

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <asm/irq.h>
#include <asm/mvtfs.h>


/*
 * We safe the current state of active irqs here so we can
 * load the active irqs from here.
 *
 * This needs be done so we can update the psw on each context
 * switch and return so all threads run with the correct irqs
 * masked
 */
unsigned long volatile irqmask = 0;


static void eco32_unmask_irq(struct irq_data* data)
{
    unsigned long psw;
    
    irqmask |= (1 << data->hwirq);

    psw = __eco32_read_psw();
    psw |= (1 << data->hwirq);
    __eco32_write_psw(psw);
}


static void eco32_mask_irq(struct irq_data* data)
{
    unsigned long psw;

    irqmask &= ~(1 << data->hwirq);
    
    psw = __eco32_read_psw();
    psw &= ~(1 << data->hwirq);
    __eco32_write_psw(psw);
}


static struct irq_chip eco32_intc = {
    .name = "ECO32",
    .irq_mask = eco32_mask_irq,
    .irq_unmask = eco32_unmask_irq,
};


int eco32_irq_map(struct irq_domain *h, unsigned int irq,
                  irq_hw_number_t hw_irq_num)
{
    irq_set_chip_and_handler(irq, &eco32_intc, handle_level_irq);

    return 0;
}

static const struct irq_domain_ops irq_ops = {
       .map    = eco32_irq_map,
       .xlate  = irq_domain_xlate_onecell,
};


static int __init eco32_intc_of_init(struct device_node *intc,
                                     struct device_node *parent)
{
    struct irq_domain *domain;

    domain = irq_domain_add_linear(intc, NR_IRQS, &irq_ops, NULL);
    BUG_ON(!domain);
    irq_set_default_host(domain);

    return 0;
}
IRQCHIP_DECLARE(eco32_intc, "thm,eco32-intc", eco32_intc_of_init);
