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
 * eco32disk.c -- the ECO32 disk driver
 */


#include <linux/fs.h>
#include <linux/module.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/errno.h>
#include <linux/blkdev.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <asm/irq.h>
#include <asm/devprobe.h>


#define ECO32DISK_SECTOR_SHIFT	9
#define ECO32DISK_SECTOR_SIZE	(1 << ECO32DISK_SECTOR_SHIFT)

#define ECO32DISK_CTL			0
#define ECO32DISK_CTL_INIT		0x20
#define ECO32DISK_CTL_FIN		0x10
#define ECO32DISK_CTL_ERR		0x08
#define ECO32DISK_CTL_WR		0x04
#define ECO32DISK_CTL_IEN		0x02
#define ECO32DISK_CTL_START		0x01

#define ECO32DISK_SCR			1
#define ECO32DISK_SAR			2
#define ECO32DISK_CR			3

#define ECO32DISK_DB			(((char*)(eco32disk_dev.base)) + 0x00080000)
#define ECO32DISK_DB_SIZE		(ECO32DISK_SECTOR_SIZE * 8)


#define ECO32DISK_DEV_NAME		"hda"
#define ECO32DISK_DRV_NAME		"eco32disk"
#define ECO32DISK_MAJOR			3
#define ECO32DISK_MINORS		16


/*
 * private struct that holds all crucial information about the driver
 */
static struct {
	unsigned int size;
	struct gendisk* gd;
	struct request_queue* queue;
	spinlock_t q_lock;
	struct request* current_request;
	int busy;
	spinlock_t lock;
	int open;
	unsigned int* base;
	int irq;
	struct device* dev;
} eco32disk_dev;


/**************************************************************/

/*
 * Disk register read/write
 */

static inline unsigned int eco32disk_in(unsigned int offset)
{
	return ioread32be(eco32disk_dev.base + offset);
}

static inline void eco32disk_out(unsigned int value, unsigned int offset)
{
	iowrite32be(value, eco32disk_dev.base + offset);
}


/**************************************************************/

/*
 * local memcpy - block buffer needs word access
 */

static inline void __wmemcpy(void* d, void* s, size_t n)
{
	unsigned int* _d = (unsigned int*)d;
	unsigned int* _s = (unsigned int*)s;

	while (n) {
		*_d++ = *_s++;
		n -= 4;
	}
}


/**************************************************************/

/*
 * Disk interrupt handler
 */

static irqreturn_t eco32disk_interrupt_handler(int irq, void* dev)
{

	unsigned long flags;
	unsigned int ctrl;
	irqreturn_t ret = IRQ_NONE;

	spin_lock_irqsave(&eco32disk_dev.q_lock, flags);

	ctrl = eco32disk_in(ECO32DISK_CTL);
	/* reset IEN flag */
	eco32disk_out(0, ECO32DISK_CTL);

	/* if there was an io error mark the request as completet with io error */
	if (ctrl & ECO32DISK_CTL_ERR) {
		__blk_end_request_all(eco32disk_dev.current_request, BLK_STS_IOERR);
		goto out_unlock_start;
	}

	/*
	 * if it was a read request fetch the data from the buffer and write
	 * the data to the segments of the request
	 */
	if (rq_data_dir(eco32disk_dev.current_request) == READ) {
		struct req_iterator iter;
		struct bio_vec bvec;
		char* buf;
		unsigned long _flags;
		unsigned int hw_offset = 0;

		rq_for_each_segment(bvec, eco32disk_dev.current_request, iter) {
			buf = bvec_kmap_irq(&bvec, &_flags);

			__wmemcpy(buf, ECO32DISK_DB + hw_offset, bvec.bv_len);

			bvec_kunmap_irq(buf, &_flags);
			hw_offset += bvec.bv_len;
		}
	}

	/* we are free again - mark request as completed */
	eco32disk_dev.busy = 0;
	__blk_end_request_all(eco32disk_dev.current_request, BLK_STS_OK);
	ret = IRQ_HANDLED;

out_unlock_start:
	blk_start_queue(eco32disk_dev.queue);
	spin_unlock_irqrestore(&eco32disk_dev.q_lock, flags);
	return ret;
}


/**************************************************************/

/*
 * Block operations
 */

static int eco32disk_open(struct block_device* bdev, fmode_t fmode)
{

	unsigned long flags;

	spin_lock_irqsave(&eco32disk_dev.lock, flags);

	/* set isr and request irq if we are the first to open */
	if (!eco32disk_dev.open++) {
		/* set isr for disk */
		set_ISR(eco32disk_dev.irq, do_IRQ);

		/* request irq */
		if (request_irq(eco32disk_dev.irq, eco32disk_interrupt_handler, 0, ECO32DISK_DEV_NAME, NULL)) {
			dev_err(eco32disk_dev.dev, "could not request irq for eco32disk\n");
			goto out_reset_isr;
		}
	}

	spin_unlock_irqrestore(&eco32disk_dev.lock, flags);

	return 0;

out_reset_isr:
	set_ISR(eco32disk_dev.irq, def_xcpt_handler);
	spin_unlock_irqrestore(&eco32disk_dev.lock, flags);
	return -EIO;
}

static void eco32disk_release(struct gendisk* gd, fmode_t fmode)
{

	unsigned long flags;

	spin_lock_irqsave(&eco32disk_dev.lock, flags);

	/* free irq and reset isr if we are the last to close */
	if (!--eco32disk_dev.open) {
		/* free irq and reset isr */
		free_irq(eco32disk_dev.irq, NULL);
		set_ISR(eco32disk_dev.irq, def_xcpt_handler);
	}

	spin_unlock_irqrestore(&eco32disk_dev.lock, flags);
}

static int eco32disk_ioctl(struct block_device* bdev, fmode_t fmode, unsigned int cmd, unsigned long arg)
{
	/* nothing to do here. we do not support special io control */
	return 0;
}

static unsigned int eco32disk_check_events(struct gendisk* gd, unsigned int clearing)
{
	/* the disk will not change or eject! */
	return 0;
}

static int eco32disk_revalidate_disk(struct gendisk* gd)
{
	/* we never change the disk and the kernel ignores our return value anyways */
	return 0xBAADF00D;
}

static const struct block_device_operations eco32disk_ops = {
	.owner = THIS_MODULE,
	.open = eco32disk_open,
	.release = eco32disk_release,
	.ioctl = eco32disk_ioctl,
	.check_events = eco32disk_check_events,
	.revalidate_disk = eco32disk_revalidate_disk,
};

/**************************************************************/

/*
 * Managing the io requests to the device
 */

static void eco32disk_request_fn(struct request_queue* q)
{

	sector_t sector;
	unsigned int nsectors;
	unsigned int ctrl = 0;

	/*
	 * we got called while being busy or stopped
	 * mark us as stopped again if someone marked us as not stopped
	 */
	if (eco32disk_dev.busy || blk_queue_stopped(q)) {
		blk_stop_queue(q);
		return;
	}

	/* fetch request from queue and set it as current active */
	if ((eco32disk_dev.current_request = blk_fetch_request(q)) == NULL) {
		return;
	}

	/* filter unsupported request operations */
	switch (req_op(eco32disk_dev.current_request)) {
		case REQ_OP_READ:
		case REQ_OP_WRITE:
			break;

		default: {
			dev_err(eco32disk_dev.dev, "unsupported operation: 0x%08x\n", req_op(eco32disk_dev.current_request));
			__blk_end_request_all(eco32disk_dev.current_request, BLK_STS_IOERR);
			return;
		}
	}

	sector = blk_rq_pos(eco32disk_dev.current_request);
	nsectors = blk_rq_bytes(eco32disk_dev.current_request) >> ECO32DISK_SECTOR_SHIFT;

	/* check if read or write would be outside of device size or exceed buffer */
	if (unlikely((sector + (nsectors >> ECO32DISK_SECTOR_SHIFT)) > eco32disk_dev.size)) {
		dev_err(eco32disk_dev.dev, "request outside device size\n");
		__blk_end_request_all(eco32disk_dev.current_request, BLK_STS_IOERR);
		return;
	}

	if (unlikely(nsectors > 8)) {
		dev_err(eco32disk_dev.dev, "request exceeds disk buffer\n");
		__blk_end_request_all(eco32disk_dev.current_request, BLK_STS_IOERR);
		return;
	}

	/* no more requests till the current one is done */
	blk_stop_queue(q);
	eco32disk_dev.busy = 1;

	/* prepare hardware to write */
	if (rq_data_dir(eco32disk_dev.current_request) == WRITE) {
		struct req_iterator iter;
		struct bio_vec bvec;
		char* buf;
		unsigned long flags;
		unsigned int hw_offset = 0;

		/* fetch data from all the io segments and copy them to the disk buffer */
		rq_for_each_segment(bvec, eco32disk_dev.current_request, iter) {
			buf = bvec_kmap_irq(&bvec, &flags);

			__wmemcpy(ECO32DISK_DB + hw_offset, buf, bvec.bv_len);

			bvec_kunmap_irq(buf, &flags);
			hw_offset += bvec.bv_len;
		}

		ctrl |= ECO32DISK_CTL_WR;
	} else {
		/* copy from buffer to segments is done in the isr */
		ctrl &= ~ECO32DISK_CTL_WR;
	}

	eco32disk_out((unsigned int)sector, ECO32DISK_SAR);
	eco32disk_out((unsigned int)nsectors, ECO32DISK_SCR);

	ctrl |= ECO32DISK_CTL_IEN | ECO32DISK_CTL_START;
	eco32disk_out(ctrl, ECO32DISK_CTL);
}

/**************************************************************/

/*
 * Module setup and teardown
 */

static int eco32disk_probe(struct platform_device* dev)
{
	int err;
	unsigned int capacity;
	unsigned int base, len;

	eco32disk_dev.dev = &dev->dev;

	/* read device node and obtain needed properties */
	if (dev->dev.of_node) {
		struct device_node* np = dev->dev.of_node;

		err = of_property_read_u32_index(np, "reg", 0, &base);

		if (err)
			goto out_no_property;

		eco32disk_dev.base = (unsigned int*)(base | PAGE_OFFSET);

		err = of_property_read_u32_index(np, "reg", 1, &len);

		if (err)
			goto out_no_property;

		err = of_property_read_u32_index(np, "interrupts", 0, &eco32disk_dev.irq);

		if (err)
			goto out_no_property;

	} else {
		dev_err(&dev->dev, "device node not present\n");
		return -ENODEV;
	}

	/* probe if device is present on the bus */
	if (eco32_device_probe((unsigned long)eco32disk_dev.base)) {
		dev_err(&dev->dev, "device not present on the bus\n");
		return -ENODEV;
	}

	/* check if device is initialized and ready */
	if (!(eco32disk_in(ECO32DISK_CTL) & ECO32DISK_CTL_INIT)) {
		if (eco32disk_in(ECO32DISK_CR) == 0) {
			dev_info(&dev->dev, "no disk installed");
			return -ENODEV;
		} else {
			dev_err(&dev->dev, "device is not ready\n");
			return -EBUSY;
		}
	}
	
	/* reset struct values */
	spin_lock_init(&eco32disk_dev.lock);
	eco32disk_dev.busy = 0;
	eco32disk_dev.open = 0;
	eco32disk_dev.current_request = NULL;

	/* register mem region */
	if (devm_request_mem_region(&dev->dev, base, len, ECO32DISK_DEV_NAME) == NULL) {
		dev_err(&dev->dev, "could not request mem region\n");
		return -EBUSY;
	}

	/* register device to block layer */
	if ((err = register_blkdev(ECO32DISK_MAJOR, ECO32DISK_DEV_NAME)) < 0) {
		dev_err(&dev->dev, "could not register block device\n");
		return err;
	}

	/* allocate gendisk struct */
	if ((eco32disk_dev.gd = alloc_disk(ECO32DISK_MINORS)) == NULL) {
		dev_err(&dev->dev, "could not allocate gendisk\n");
		err = -ENOMEM;
		goto out_unregister;
	}

	/*
	 * init spinlock for driver request queue
	 * the lock must be hold an all operations on the queue
	 */
	spin_lock_init(&eco32disk_dev.q_lock);

	/* init request queue */
	if ((eco32disk_dev.queue = blk_init_queue(eco32disk_request_fn, &eco32disk_dev.q_lock)) == NULL) {
		dev_err(&dev->dev, "could not initialize request queue\n");
		err = -ENOMEM;
		goto out_free;
	}

	/*
	 * set gendisk up
	 * add it to black layer as last step because the kernel will
	 * instantly want to read from the disk but we are not ready yet
	 */
	eco32disk_dev.gd->major = ECO32DISK_MAJOR;
	eco32disk_dev.gd->first_minor = 1;
	eco32disk_dev.gd->queue = eco32disk_dev.queue;
	eco32disk_dev.gd->fops = &eco32disk_ops;
	strcpy(eco32disk_dev.gd->disk_name, ECO32DISK_DEV_NAME);
	capacity = eco32disk_in(ECO32DISK_CR);
	set_capacity(eco32disk_dev.gd, capacity);

	eco32disk_dev.size = capacity;

	/*
	 * let the block layer know how our hardware works and how much at a
	 * time we can handle
	 * 
	 * 8 is the minimum. luckily this is exactly the size of our data buffer
	 * so we can fullfill one request at a time without addition handling :)
	 */
	blk_queue_max_hw_sectors(eco32disk_dev.queue, 8);
	blk_queue_logical_block_size(eco32disk_dev.queue, ECO32DISK_SECTOR_SIZE);

	/* finally add disk */
	add_disk(eco32disk_dev.gd);

	return 0;
out_no_property:
	dev_err(&dev->dev, "could not read all necessary properties from device tree\n");
	return -ENODEV;
out_free:
	del_gendisk(eco32disk_dev.gd);
out_unregister:
	unregister_blkdev(ECO32DISK_MAJOR, ECO32DISK_DEV_NAME);
	return err;
}


static int eco32disk_remove(struct platform_device* dev)
{
	unregister_blkdev(ECO32DISK_MAJOR, ECO32DISK_DEV_NAME);
	del_gendisk(eco32disk_dev.gd);
	blk_cleanup_queue(eco32disk_dev.queue);
	return 0;
}

static struct of_device_id eco32disk_of_ids[] = {
	{
		.compatible = "thm,eco32-disk",
	},
	{0},
};

static struct platform_driver eco32disk_platform_driver = {
	.probe = eco32disk_probe,
	.remove = eco32disk_remove,
	.driver = {
		.name = ECO32DISK_DRV_NAME,
		.of_match_table = of_match_ptr(eco32disk_of_ids),
	},
};
module_platform_driver(eco32disk_platform_driver);

MODULE_LICENSE("GPL");
