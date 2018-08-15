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
 * eco32fb.c -- the ECO32 framebuffer driver
 */


#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/fb.h>

#include <asm/devprobe.h>


#define ECO32GRAPHIC_DRV_NAME		"eco32graphic"

#define ECO32GRAPHIC_WIDTH			640
#define ECO32GRAPHIC_HEIGHT			480


static struct {
	unsigned int* base;
	unsigned int len;
	struct fb_info* info;
	uint32_t pseudo_palette[16];
} eco32fb_dev;


struct fb_var_screeninfo eco32fb_var = {
	.xres		= ECO32GRAPHIC_WIDTH,
	.yres		= ECO32GRAPHIC_HEIGHT,
	.xres_virtual	= ECO32GRAPHIC_WIDTH,
	.yres_virtual	= ECO32GRAPHIC_HEIGHT,
	.bits_per_pixel	= 32,
	.height		= -1,
	.width		= -1,
	.vmode		= FB_VMODE_NONINTERLACED,
	.red = {
		.length = 8,
		.offset = 16,
	},
	.green = {
		.length = 8,
		.offset = 8,
	},
	.blue = {
		.length = 8,
		.offset = 0,
	},
};

static struct fb_fix_screeninfo eco32fb_fix = {
	.id		= ECO32GRAPHIC_DRV_NAME,
	.type		= FB_TYPE_PACKED_PIXELS,
	.visual		= FB_VISUAL_TRUECOLOR,
	.line_length	= ECO32GRAPHIC_WIDTH*4,
};


/**************************************************************/

/*
 * Framebuffer operations
 */


static int eco32fb_blank(int blank, struct fb_info* info)
{
	int i;

	for (i = 0; i < ECO32GRAPHIC_WIDTH*ECO32GRAPHIC_HEIGHT; i++) {
		*(eco32fb_dev.base+i) = 0x00000000;
	}

	return 0;
}

static int eco32fb_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info* info)
{

	if (regno >= 16) {
		return 1;
	}

	red >>= 8;
	green >>= 8;
	blue >>= 8;
	transp >>= 8;

	((__be32*)info->pseudo_palette)[regno] = cpu_to_be32(transp << 24 | red << 16 | green << 8 | blue << 0);

	return 0;
}

static struct fb_ops eco32fb_ops = {
	.owner = THIS_MODULE,
	.fb_blank = eco32fb_blank,
	.fb_imageblit = cfb_imageblit,
	.fb_fillrect = cfb_fillrect,
	.fb_setcolreg = eco32fb_setcolreg,
};


/**************************************************************/

/*
 * Module setup and teardown
 */


static int eco32fb_probe(struct platform_device* dev)
{
	int err;
	unsigned int base;

	/* read device node and obtain needed properties */
	if (dev->dev.of_node) {
		struct device_node* np = dev->dev.of_node;

		err = of_property_read_u32_index(np, "reg", 0, &base);

		if (err)
			goto out_no_property;

		eco32fb_dev.base = (unsigned int*)(base | PAGE_OFFSET);

		err = of_property_read_u32_index(np, "reg", 1, &eco32fb_dev.len);

		if (err)
			goto out_no_property;

	} else {
		dev_err(&dev->dev, "device node not present\n");
		return -ENODEV;
	}


	if (eco32_device_probe((unsigned long)eco32fb_dev.base)) {
		dev_err(&dev->dev, "device not present on the bus\n");
		return -ENODEV;
	}

	/*request mem region */
	if (devm_request_mem_region(&dev->dev, base, eco32fb_dev.len, "graphicsECO") == NULL) {
		dev_err(&dev->dev, "could not request mem region\n");
		return -EBUSY;
	}

	/* allocate framebuffer device and fill it */
	eco32fb_dev.info = framebuffer_alloc(0, &dev->dev);

	if (!eco32fb_dev.info) {
		dev_err(&dev->dev, "could not allocate framebuffer device\n");
		return -ENOMEM;
	}

	eco32fb_dev.info->fbops = &eco32fb_ops;
	eco32fb_fix.smem_start = (unsigned long)eco32fb_dev.base;
	eco32fb_fix.smem_len = eco32fb_dev.len;
	eco32fb_dev.info->fix = eco32fb_fix;
	eco32fb_dev.info->var = eco32fb_var;
	eco32fb_dev.info->screen_base = (u_char*)eco32fb_dev.base;

	err = fb_alloc_cmap(&eco32fb_dev.info->cmap, 256, 0);

	if (err) {
		dev_err(&dev->dev, "could not allocate color map\n");
		err = -ENOMEM;
		goto out_release;
	}

	eco32fb_dev.info->pseudo_palette = &eco32fb_dev.pseudo_palette;

	err = register_framebuffer(eco32fb_dev.info);

	if (err) {
		dev_err(&dev->dev, "could not register framebuffer device\n");
		err = -EIO;
		goto out_dealloc;
	}

	return 0;
out_no_property:
	dev_err(&dev->dev, "could not read all necessary properties from device tree\n");
	return -ENODEV;
out_dealloc:
	fb_dealloc_cmap(&eco32fb_dev.info->cmap);
out_release:
	framebuffer_release(eco32fb_dev.info);
	return err;
}

static int eco32fb_remove(struct platform_device* dev)
{

	unregister_framebuffer(eco32fb_dev.info);
	fb_dealloc_cmap(&eco32fb_dev.info->cmap);
	framebuffer_release(eco32fb_dev.info);
	return 0;
}

static struct of_device_id eco32fb_of_ids[] = {
	{
		.compatible = "thm,eco32-graphic",
	},
	{0},
};

static struct platform_driver eco32fb_platform_driver = {
	.probe = eco32fb_probe,
	.remove = eco32fb_remove,
	.driver = {
		.name = ECO32GRAPHIC_DRV_NAME,
		.of_match_table = of_match_ptr(eco32fb_of_ids),
	},
};
module_platform_driver(eco32fb_platform_driver);

MODULE_LICENSE("GPL");
