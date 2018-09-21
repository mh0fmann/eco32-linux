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
 * Probe if the given address is present on the bus
 * 
 * This is used by all eco32 drivers to check if the
 * deive is present on the bus.
 * 
 * NOTE: this function turns off irqs while it is running.
 * this is neccessary to savely change ISR for BUS_TIMEOUT_EXCEPTION
 */
extern int eco32_device_probe(unsigned long address);
