/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eco32kbd.c -- the ECO32 keyboard driver
 */


#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/input.h>

#include <asm/irq.h>
#include <asm/devprobe.h>


#define ECO32KEYBOARD_DRV_NAME		"eco32keyboard"
#define ECO32KEYBOARD_DEV_NAME		"keyboardECO"

#define ECO32KEYBOARD_CTRL			0
#define ECO32KEYBOARD_CTRL_IEN		0x2

#define ECO32KEYBOARD_DATA			1

#define ECO32KEYBOARD_KEY_MASK		0x7F
#define ECO32KEYBOARD_KEY_EXTENDED	0xE0
#define ECO32KEYBOARD_EXTENDED_MASK	0x80
#define ECO32KEYBOARD_KEY_RELEASE	0xF0
#define ECO32KEYBOARD_MAX_SCANCODES	256


static struct {
	int irq;
	struct input_dev* input;
	unsigned int __iomem* base;
} eco32keyboard_dev;

static unsigned short keycodes[ECO32KEYBOARD_MAX_SCANCODES] = {
	/*
	 * Since the eco32 does not have a dedicated keyboard controller we recieve
	 * raw make and break codes from the ps2 keyboard connnected to the fpga.
	 * This means our keycodes can be obtained from this table that lists all
	 * make and break codes for standard ps2 keyboards:
	 * https://www.marjorie.de/ps2/scancode-set2.htm
	 */
	//Printscreen is awefull long..
	//Pause is even longer..
	/*  0 -  7*/	0, KEY_F9, 0, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12,
	/*  8 -  F*/	0, KEY_F10, KEY_F8, KEY_F6, KEY_F4, KEY_TAB, KEY_GRAVE, 0,
	/* 10 - 17*/	0, KEY_LEFTALT, KEY_LEFTSHIFT, 0, KEY_LEFTCTRL, KEY_Q, KEY_1, 0,
	/* 18 - 1F*/	0, 0, KEY_Z, KEY_S, KEY_A, KEY_W, KEY_2, KEY_LEFTMETA,
	/* 20 - 27*/	0, KEY_C, KEY_X, KEY_D, KEY_E, KEY_4, KEY_3, KEY_RIGHTMETA,
	/* 28 - 2F*/	0, KEY_SPACE, KEY_V, KEY_F, KEY_T, KEY_R, KEY_5, 0,
	/* 30 - 37*/	0, KEY_N, KEY_B, KEY_H, KEY_G, KEY_Y, KEY_6, 0,
	/* 38 - 3F*/	0, 0, KEY_M, KEY_J, KEY_U, KEY_7, KEY_8, 0,
	/* 40 - 47*/	0, KEY_COMMA, KEY_K, KEY_I, KEY_O, KEY_0, KEY_9, 0,
	/* 48 - 4F*/	0, KEY_DOT, KEY_SLASH, KEY_L, KEY_SEMICOLON, KEY_P, KEY_MINUS, 0,
	/* 50 - 57*/	0, 0, KEY_APOSTROPHE, 0, KEY_LEFTBRACE, KEY_EQUAL, 0, 0,
	/* 58 - 5F*/	KEY_CAPSLOCK, KEY_RIGHTSHIFT, KEY_ENTER, KEY_RIGHTBRACE, 0, KEY_BACKSLASH, 0, 0,
	/* 60 - 67*/	0, KEY_102ND, 0, 0, 0, 0, KEY_BACKSPACE, 0,
	/* 68 - 6F*/	0, KEY_KP1, 0, KEY_KP4, KEY_KP7, 0, 0, 0,
	/* 70 - 77*/	KEY_KP0, KEY_KPDOT, KEY_KP2, KEY_KP5, KEY_KP6, KEY_KP8, KEY_ESC, KEY_NUMLOCK,
	/* 78 - 7F*/	KEY_F11, KEY_KPPLUS, KEY_KP3, KEY_KPMINUS, KEY_KPASTERISK, KEY_KP9, KEY_SCROLLLOCK, 0,
	/* -- EXTENDED -- */
	/*  0 -  7*/	0, 0, KEY_F7, 0, 0, 0, 0, 0, //KEY_F7 is not an extende key but uses 83 as scancode and so reaches within the extended key range
	/*  8 -  F*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 10 - 17*/	0, KEY_RIGHTALT, 0, 0, KEY_RIGHTCTRL, 0, 0, 0,
	/* 18 - 1F*/	0, 0, 0, 0, 0, 0, 0, KEY_LEFTMETA,
	/* 20 - 27*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 28 - 2F*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 30 - 37*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 38 - 3F*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 40 - 47*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 48 - 4F*/	0, 0, KEY_KPSLASH, 0, 0, 0, 0, 0,
	/* 50 - 57*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 58 - 5F*/	0, 0, KEY_KPENTER, 0, 0, 0, 0, 0,
	/* 60 - 67*/	0, 0, 0, 0, 0, 0, 0, 0,
	/* 68 - 6F*/	0, KEY_END, 0, KEY_LEFT, KEY_HOME, 0, 0, 0,
	/* 70 - 77*/	KEY_INSERT, KEY_DELETE, KEY_DOWN, 0, KEY_RIGHT, KEY_UP, 0, 0,
	/* 78 - 7F*/	0, 0, KEY_PAGEDOWN, 0, 0, KEY_PAGEUP, 0, 0
};



/**************************************************************/

/*
 * Keyboard register read/write
 */

static inline unsigned int eco32keyboard_in(unsigned int offset)
{
	return ioread32be(eco32keyboard_dev.base + offset);
}

static inline void eco32keyboard_out(unsigned int value, unsigned int offset)
{
	iowrite32be(value, eco32keyboard_dev.base + offset);
}


/**************************************************************/

/*
 * Interrupt handler
 */

static irqreturn_t eco32keyboard_interrupt_handler(int irq, void* dev)
{
	unsigned char code;
	static int pressed = 1;
	static int extended = 0;

	code = eco32keyboard_in(ECO32KEYBOARD_DATA);

	switch (code) {
		case ECO32KEYBOARD_KEY_EXTENDED: {
			extended = 1;
			break;
		}

		case ECO32KEYBOARD_KEY_RELEASE: {
			pressed = 0;
			break;
		}

		default: {
			if (extended) {
				code |= ECO32KEYBOARD_EXTENDED_MASK;
			}

			if (keycodes[code]) {
				input_report_key(eco32keyboard_dev.input, keycodes[code], pressed);
				input_sync(eco32keyboard_dev.input);
				pressed = 1;
				extended = 0;
			}
		}
	}

	return IRQ_HANDLED;
}

/**************************************************************/

/*
 * Module setup and teardown
 */


static int eco32keyboard_probe(struct platform_device* dev)
{
	int err, i;
	unsigned int base, len;

	/* read device node and obtain needed properties */
	if (dev->dev.of_node) {
		struct device_node* np = dev->dev.of_node;

		err = of_property_read_u32_index(np, "reg", 0, &base);

		if (err)
			goto out_no_property;

		eco32keyboard_dev.base = (unsigned int*)(base | PAGE_OFFSET);

		err = of_property_read_u32_index(np, "reg", 1, &len);

		if (err)
			goto out_no_property;

		err = of_property_read_u32_index(np, "interrupts", 0, &eco32keyboard_dev.irq);

		if (err)
			goto out_no_property;

	} else {
		dev_err(&dev->dev, "device node not present");
		return -ENODEV;
	}


	if (eco32_device_probe((unsigned long)eco32keyboard_dev.base)) {
		dev_err(&dev->dev, "device not present on the bus\n");
		return -ENODEV;
	}

	/* request mem region */
	if (devm_request_mem_region(&dev->dev, base, len, ECO32KEYBOARD_DEV_NAME) == NULL) {
		dev_err(&dev->dev, "could not request mem region\n");
		return -EBUSY;
	}

	/* allocate input device and fill it */
	eco32keyboard_dev.input = devm_input_allocate_device(&dev->dev);

	if (!eco32keyboard_dev.input) {
		dev_err(&dev->dev, "could not allocate input device for eco32 keyboard\n");
		return -ENOMEM;
	}

	eco32keyboard_dev.input->name = ECO32KEYBOARD_DEV_NAME;
	eco32keyboard_dev.input->phys = "eco32-kbd/input0";
	eco32keyboard_dev.input->keycode = keycodes;
	eco32keyboard_dev.input->keycodesize = sizeof(keycodes[0]);
	eco32keyboard_dev.input->keycodemax = ECO32KEYBOARD_MAX_SCANCODES;
	eco32keyboard_dev.input->id.bustype = BUS_HOST;
	eco32keyboard_dev.input->id.vendor = 0x1;
	eco32keyboard_dev.input->id.product = 0x1;
	eco32keyboard_dev.input->id.version = 0x1;

	__set_bit(EV_KEY, eco32keyboard_dev.input->evbit);

	/* set keycodes */
	for (i = 0; i < ECO32KEYBOARD_MAX_SCANCODES; i++) {
		__set_bit(keycodes[i], eco32keyboard_dev.input->keybit);
	}

	__clear_bit(KEY_RESERVED, eco32keyboard_dev.input->keybit);

	/* set isr */
	set_ISR(eco32keyboard_dev.irq, do_IRQ);

	/* request irq */
	if (devm_request_irq(&dev->dev, eco32keyboard_dev.irq, eco32keyboard_interrupt_handler, 0, ECO32KEYBOARD_DEV_NAME, NULL)) {
		dev_err(&dev->dev, "could not request irq for eco32 keyboard\n");
		set_ISR(eco32keyboard_dev.irq, def_xcpt_handler);
		return -EIO;
	}

	err = input_register_device(eco32keyboard_dev.input);

	if (err) {
		dev_err(&dev->dev, "could not register eco32 keyboard input device\n");
		return -EIO;
	}

	/* clear pending interrupt and scancode*/
	err = eco32keyboard_in(ECO32KEYBOARD_DATA);
	/* enable interrupts */
	eco32keyboard_out(ECO32KEYBOARD_CTRL_IEN, ECO32KEYBOARD_CTRL);

	return 0;
out_no_property:
	dev_err(&dev->dev, "could not read all necessary properties from device tree\n");
	return -ENODEV;
}

static int eco32keyboard_remove(struct platform_device* dev)
{

	eco32keyboard_out(ECO32KEYBOARD_CTRL, 0x00);
	set_ISR(eco32keyboard_dev.irq, def_xcpt_handler);
	input_unregister_device(eco32keyboard_dev.input);
	return 0;
}

static struct of_device_id eco32keyboard_of_ids[] = {
	{
		.compatible = "thm,eco32-keyboard",
	},
	{0},
};

static struct platform_driver eco32keyboard_platform_driver = {
	.probe = eco32keyboard_probe,
	.remove = eco32keyboard_remove,
	.driver = {
		.name = ECO32KEYBOARD_DRV_NAME,
		.of_match_table = of_match_ptr(eco32keyboard_of_ids),
	},
};
module_platform_driver(eco32keyboard_platform_driver);


MODULE_LICENSE("GPL");
