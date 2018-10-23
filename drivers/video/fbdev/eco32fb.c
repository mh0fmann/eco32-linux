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


#define ECO32GRAPHIC_DRV_NAME       "eco32graphic"

#define ECO32GRAPHIC_WIDTH          640
#define ECO32GRAPHIC_HEIGHT         480


static struct fb_info* info;
static uint32_t pseudo_palette[16];


struct fb_var_screeninfo eco32fb_var = {
    .xres           = ECO32GRAPHIC_WIDTH,
    .yres           = ECO32GRAPHIC_HEIGHT,
    .xres_virtual   = ECO32GRAPHIC_WIDTH,
    .yres_virtual   = ECO32GRAPHIC_HEIGHT,
    .bits_per_pixel = 32,
    .height         = -1,
    .width          = -1,
    .vmode          = FB_VMODE_NONINTERLACED,
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
    .id             = ECO32GRAPHIC_DRV_NAME,
    .type           = FB_TYPE_PACKED_PIXELS,
    .visual         = FB_VISUAL_TRUECOLOR,
    .line_length    = ECO32GRAPHIC_WIDTH*4,
};


/**************************************************************/

/*
 * Framebuffer operations
 */


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
    unsigned long base;
    unsigned long len;
    struct device_node * node = dev->dev.of_node;

    /* read device node and obtain needed properties */
    if (node) {
        base = (unsigned long)of_iomap(node, 0);

        err = of_property_read_u32_index(node, "reg", 1, (unsigned int*) &len);

        if (err)
            goto out_no_property;

    } else {
        dev_err(&dev->dev, "device node not present\n");
        return -ENODEV;
    }


    if (eco32_device_probe(base)) {
        dev_err(&dev->dev, "device not present on the bus\n");
        return -ENODEV;
    }

    /*request mem region */
    if (devm_request_mem_region(&dev->dev, base, len, "fbECO") == NULL) {
        dev_err(&dev->dev, "could not request mem region\n");
        return -EBUSY;
    }

    /* allocate framebuffer device and fill it */
    info = framebuffer_alloc(0, &dev->dev);

    if (!info) {
        dev_err(&dev->dev, "could not allocate framebuffer device\n");
        return -ENOMEM;
    }

    info->fbops = &eco32fb_ops;
    eco32fb_fix.smem_start = base;
    eco32fb_fix.smem_len = len;
    info->fix = eco32fb_fix;
    info->var = eco32fb_var;
    info->screen_base = (u_char*)base;

    err = fb_alloc_cmap(&info->cmap, 256, 0);

    if (err) {
        dev_err(&dev->dev, "could not allocate color map\n");
        err = -ENOMEM;
        goto out_release;
    }

    info->pseudo_palette = pseudo_palette;

    err = register_framebuffer(info);

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
    fb_dealloc_cmap(&info->cmap);
out_release:
    framebuffer_release(info);
    return err;
}

static int eco32fb_remove(struct platform_device* dev)
{
    unregister_framebuffer(info);
    fb_dealloc_cmap(&info->cmap);
    framebuffer_release(info);
    return 0;
}

static struct of_device_id eco32fb_of_ids[] = {
    {
        .compatible = "thm,eco32-graphic",
    },
    {0},
};

static struct platform_driver eco32fb_platform_driver = {
    .probe  = eco32fb_probe,
    .remove = eco32fb_remove,
    .driver = {
        .name           = ECO32GRAPHIC_DRV_NAME,
        .of_match_table = of_match_ptr(eco32fb_of_ids),
    },
};
module_platform_driver(eco32fb_platform_driver);

MODULE_LICENSE("GPL");
