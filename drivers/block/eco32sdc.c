/*
 * ECO32 Linux
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for ECO32:
 * Copyright (c) 2018 Hellwig Geisse, Martin Hofmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * eco32sdc.c -- the ECO32 sdcard driver
 */


#include <linux/fs.h>
#include <linux/module.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/errno.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <asm/devprobe.h>


/* SDC addresses */
#define SDC_BASE    ((volatile unsigned int*)eco32sdc_dev.base)
#define SDC_CTRL    (SDC_BASE + 0)
#define SDC_DATA    (SDC_BASE + 1)
#define SDC_CRC7    (SDC_BASE + 2)
#define SDC_CRC16   (SDC_BASE + 3)

/* SDC control bits */
#define SDC_CRC16MISO   ((unsigned int) 0x04)
#define SDC_FASTCLK ((unsigned int) 0x02)
#define SDC_SELECT  ((unsigned int) 0x01)

/* SDC status bits */
#define SDC_READY   ((unsigned int) 0x01)


#define START_BLOCK_TOKEN   0xFE



#define ECO32SDC_DRV_NAME   "eco32sdc"
#define ECO32SDC_DEV_NAME   "hdc"

#define ECO32SDC_SECTOR_SHIFT   9
#define ECO32SDC_SECTOR_SIZE    (1 << ECO32SDC_SECTOR_SHIFT)

#define ECO32SDC_MAJOR      22
#define ECO32SDC_MINORS     16


/*
 * private struct that holds all crucial information about the driver
 */
static struct {
    struct gendisk* gd;
    struct request_queue* queue;
    struct blk_mq_tag_set tag_set;
    struct request* current_request;
    unsigned int busy;
    spinlock_t lock;
    unsigned int size;
    unsigned int* base;
    struct device* dev;
    unsigned int lastSent;
} eco32sdc_dev;



/**************************************************************/

/*
 * Support functions for reading and writing to the sdcard
 */


static void select(void)
{
    eco32sdc_dev.lastSent |= SDC_SELECT;
    *SDC_CTRL = eco32sdc_dev.lastSent;
}

static void deselect(void)
{
    eco32sdc_dev.lastSent &= ~SDC_SELECT;
    *SDC_CTRL = eco32sdc_dev.lastSent;
}

static void fastClock(void)
{
    eco32sdc_dev.lastSent |= SDC_FASTCLK;
    *SDC_CTRL = eco32sdc_dev.lastSent;
}

static void slowClock(void)
{
    eco32sdc_dev.lastSent &= ~SDC_FASTCLK;
    *SDC_CTRL = eco32sdc_dev.lastSent;
}


static void resetCRC7(void)
{
    *SDC_CRC7 = 0x01;
}


static unsigned char getCRC7(void)
{
    return *SDC_CRC7;
}


static void resetCRC16(int fromMISO)
{
    if (fromMISO) {
        /* compute CRC for bits from MISO */
        eco32sdc_dev.lastSent |= SDC_CRC16MISO;
        *SDC_CTRL = eco32sdc_dev.lastSent;
    } else {
        /* compute CRC for bits from MOSI */
        eco32sdc_dev.lastSent &= ~SDC_CRC16MISO;
        *SDC_CTRL = eco32sdc_dev.lastSent;
    }

    *SDC_CRC16 = 0x0000;
}


static unsigned short getCRC16(void)
{
    return *SDC_CRC16;
}


static unsigned char sndRcv(unsigned char b)
{
    unsigned char r;

    *SDC_DATA = b;

    while (1) {
        volatile unsigned int ctrl = *SDC_CTRL;

        if ((ctrl & SDC_READY)) {
            break;
        }
    }

    r = *SDC_DATA;
    return r;
}


static int sndCmd(unsigned char* cmd,
                  unsigned char* rcv, int size)
{
    int i;
    unsigned char r;

    select();
    /* send command */
    resetCRC7();

    for (i = 0; i < 5; i++) {
        (void) sndRcv(cmd[i]);
    }

    (void) sndRcv(getCRC7());
    /* receive answer */
    i = 8;

    do {
        r = sndRcv(0xFF);
    } while (r == 0xFF && --i > 0);

    if (i == 0) {
        return 0;
    }

    rcv[0] = r;

    /* possibly receive more answer bytes */
    for (i = 1; i < size; i++) {
        rcv[i] = sndRcv(0xFF);
    }

    deselect();
    (void) sndRcv(0xFF);
    return size;
}


static void initCard(void)
{
    int i;

    slowClock();
    deselect();

    for (i = 0; i < 10; i++) {
        (void) sndRcv(0xFF);
    }
}


static int sndCmdRcvData(unsigned char* cmd,
                         unsigned char* rcv,
                         unsigned char* ptr, int size)
{
    int i;
    unsigned char r;
    unsigned short crc16;

    select();
    /* send command */
    resetCRC7();

    for (i = 0; i < 5; i++) {
        (void) sndRcv(cmd[i]);
    }

    (void) sndRcv(getCRC7());
    /* receive answer */
    i = 8;

    do {
        r = sndRcv(0xFF);
    } while (r == 0xFF && --i > 0);

    if (i == 0) {
        return 0;
    }

    rcv[0] = r;
    /* wait for start block token */
    i = 2048;

    do {
        r = sndRcv(0xFF);
    } while (r != START_BLOCK_TOKEN && --i > 0);

    if (i == 0) {
        return 0;
    }

    /* receive data bytes */
    resetCRC16(1);

    for (i = 0; i < size; i++) {
        ptr[i] = sndRcv(0xFF);
    }

    /* receive CRC */
    (void) sndRcv(0xFF);
    (void) sndRcv(0xFF);
    crc16 = getCRC16();
    deselect();
    (void) sndRcv(0xFF);

    if (crc16 != 0x0000) {
        /* CRC error */
        return 0;
    }

    return size;
}


static void resetCard(void)
{
    unsigned char cmd[5] = { 0x40, 0x00, 0x00, 0x00, 0x00 };
    unsigned char rcv[1];
    int n;

    n = sndCmd(cmd, rcv, 1);
}


static void sendInterfaceCondition(void)
{
    unsigned char cmd[5] = { 0x48, 0x00, 0x00, 0x01, 0xAA };
    unsigned char rcv[5];
    int n;

    n = sndCmd(cmd, rcv, 5);
}


static void setCrcOptionOn(void)
{
    unsigned char cmd[5] = { 0x7B, 0x00, 0x00, 0x00, 0x01 };
    unsigned char rcv[1];
    int n;

    n = sndCmd(cmd, rcv, 1);
}


static unsigned char sendHostCap(void)
{
    unsigned char cmd[5] = { 0x69, 0x40, 0x00, 0x00, 0x00 };
    unsigned char rcv[1];
    int n;

    n = sndCmd(cmd, rcv, 1);

    return n == 0 ? 0xFF : rcv[0];
}


static void prepAppCmd(void)
{
    unsigned char cmd[5] = { 0x77, 0x00, 0x00, 0x00, 0x00 };
    unsigned char rcv[1];
    int n;

    n = sndCmd(cmd, rcv, 1);
}


static int activateCard(void)
{
    int tries;
    unsigned char res;

    for (tries = 0; tries < 1000; tries++) {
        prepAppCmd();
        res = sendHostCap();

        if (res == 0x00) {
            /* SD card activated */
            return 0;
        }
    }

    /* cannot activate SD card */
    return -1;
}


static void readOCR(void)
{
    unsigned char cmd[5] = { 0x7A, 0x00, 0x00, 0x00, 0x00 };
    unsigned char rcv[5];
    int n;

    n = sndCmd(cmd, rcv, 5);
}


static void sendCardSpecData(unsigned char* csd)
{
    unsigned char cmd[5] = { 0x49, 0x00, 0x00, 0x00, 0x00 };
    unsigned char rcv[1];
    int n;

    n = sndCmdRcvData(cmd, rcv, csd, 16);
}


static int sndCmdSndData(unsigned char* cmd,
                         unsigned char* rcv,
                         unsigned char* ptr, int size)
{
    int i;
    unsigned char r;
    unsigned short crc16;

    select();
    /* send command */
    resetCRC7();

    for (i = 0; i < 5; i++) {
        (void) sndRcv(cmd[i]);
    }

    (void) sndRcv(getCRC7());
    /* receive answer */
    i = 8;

    do {
        r = sndRcv(0xFF);
    } while (r == 0xFF && --i > 0);

    if (i == 0) {
        return 0;
    }

    rcv[0] = r;
    /* send start block token */
    (void) sndRcv(START_BLOCK_TOKEN);
    /* send data bytes */
    resetCRC16(false);

    for (i = 0; i < size; i++) {
        (void) sndRcv(ptr[i]);
    }

    /* send CRC */
    crc16 = getCRC16();
    (void) sndRcv((crc16 >> 8) & 0xFF);
    (void) sndRcv((crc16 >> 0) & 0xFF);

    /* receive data respose token */
    do {
        r = sndRcv(0xFF);
    } while (r == 0xFF);

    rcv[1] = r;

    if ((r & 0x1F) != 0x05) {
        /* rejected */
        return 0;
    }

    /* wait while busy */
    do {
        r = sndRcv(0xFF);
    } while (r == 0x00);

    deselect();
    (void) sndRcv(0xFF);
    return size;
}


static int initSDC(void)
{
    initCard();
    resetCard();
    sendInterfaceCondition();
    setCrcOptionOn();

    if (!activateCard()) {
        return -1;
    }

    readOCR();
    fastClock();
    return 0;
}


static int dskcapsdc(void)
{
    unsigned char csd[16];
    unsigned int csize;
    int numSectors;

    sendCardSpecData(csd);

    if ((csd[0] & 0xC0) != 0x40) {
        /* wrong CSD structure version */
        return 0;
    }

    csize = (((unsigned int) csd[7] & 0x3F) << 16) |
            (((unsigned int) csd[8] & 0xFF) <<  8) |
            (((unsigned int) csd[9] & 0xFF) <<  0);
    numSectors = (csize + 1) << 10;
    return numSectors;
}


static void readSingleSector(unsigned int sct, unsigned char* ptr)
{
    unsigned char cmd[5] = { 0x51, 0x00, 0x00, 0x00, 0x00 };
    unsigned char rcv[1];
    int n;

    cmd[1] = (sct >> 24) & 0xFF;
    cmd[2] = (sct >> 16) & 0xFF;
    cmd[3] = (sct >>  8) & 0xFF;
    cmd[4] = (sct >>  0) & 0xFF;
    n = sndCmdRcvData(cmd, rcv, ptr, 512);
}


static void writeSingleSector(unsigned int sct, unsigned char* ptr)
{
    unsigned char cmd[5] = { 0x58, 0x00, 0x00, 0x00, 0x00 };
    unsigned char rcv[2];
    int n;

    cmd[1] = (sct >> 24) & 0xFF;
    cmd[2] = (sct >> 16) & 0xFF;
    cmd[3] = (sct >>  8) & 0xFF;
    cmd[4] = (sct >>  0) & 0xFF;
    n = sndCmdSndData(cmd, rcv, ptr, 512);
}


/**************************************************************/

/*
 * Block operations
 */

static int eco32sdc_open(struct block_device* bdev, fmode_t fmode)
{
    /* nothing to do */
    return 0;
}

static void eco32sdc_release(struct gendisk* gd, fmode_t fmode)
{
    /* nothing to do */
}

static int eco32sdc_ioctl(struct block_device* bdev, fmode_t fmode, unsigned int cmd, unsigned long arg)
{
    /* nothing to do here. we do not support special io control */
    return 0;
}

static unsigned int eco32sdc_check_events(struct gendisk* gd, unsigned int clearing)
{
    /* the disk will not change or eject! */
    return 0;
}

static int eco32sdc_revalidate_disk(struct gendisk* gd)
{
    /* we never change the disk and the kernel ignores our return value anyways */
    return 0xBAADF00D;
}

static const struct block_device_operations eco32sdc_ops = {
    .owner = THIS_MODULE,
    .open = eco32sdc_open,
    .release = eco32sdc_release,
    .ioctl = eco32sdc_ioctl,
    .check_events = eco32sdc_check_events,
    .revalidate_disk = eco32sdc_revalidate_disk,
};



/**************************************************************/

/*
 * SDcard tasklet
 */


static void eco32sdc_request_tasklet(unsigned long data)
{
    /* read or write data to or from sdcard to or from segements */
    {
        struct req_iterator iter;
        struct bio_vec bvec;
        char* buf;
        int i;
        unsigned long _flags;
        sector_t sec;
        int sectorinc = 0;
        sec = blk_rq_pos(eco32sdc_dev.current_request);

        rq_for_each_segment(bvec, eco32sdc_dev.current_request, iter) {
            buf = bvec_kmap_irq(&bvec, &_flags);

            for (i = 0; i < bvec.bv_len; i+= ECO32SDC_SECTOR_SIZE) {
                switch (rq_data_dir(eco32sdc_dev.current_request)) {
                    case READ: {
                        readSingleSector(sec+sectorinc, buf+i);
                    }

                    case WRITE: {
                        writeSingleSector(sec+sectorinc, buf+i);
                    }
                }

                sectorinc++;
            }

            bvec_kunmap_irq(buf, &_flags);
        }
    }

    __blk_mq_end_request(eco32sdc_dev.current_request, BLK_STS_OK);

    /* mark as free and start queue */
    eco32sdc_dev.busy = 0;
    blk_mq_start_hw_queues(eco32sdc_dev.queue);

}
DECLARE_TASKLET(eco32sdc_tasklet, eco32sdc_request_tasklet, 0);


/**************************************************************/

/*
 * Managing the io requests to the device
 */

static blk_status_t eco32sdc_queue_rq(struct blk_mq_hw_ctx *hctx,
                              const struct blk_mq_queue_data *bd)
{
    sector_t sector;
    unsigned int nsectors;

    /*
     * we got called while being busy or stopped
     * mark us as stopped again if someone marked us as not stopped
     */
    if (eco32sdc_dev.busy || blk_mq_queue_stopped(eco32sdc_dev.queue)) {
        blk_mq_stop_hw_queues(eco32sdc_dev.queue);
        return BLK_STS_IOERR;;
    }

    eco32sdc_dev.current_request = bd->rq;

    /* filter unsupported request operations */
    switch (req_op(eco32sdc_dev.current_request)) {
        case REQ_OP_READ:
        case REQ_OP_WRITE:
            break;

        default: {
            dev_err(eco32sdc_dev.dev, "unsupported operation: 0x%08x\n", req_op(eco32sdc_dev.current_request));
            __blk_mq_end_request(eco32sdc_dev.current_request, BLK_STS_IOERR);
            return BLK_STS_IOERR;;
        }
    }

    /* fetch data for sanity checks */
    sector = blk_rq_pos(eco32sdc_dev.current_request);
    nsectors = blk_rq_bytes(eco32sdc_dev.current_request) >> ECO32SDC_SECTOR_SHIFT;

    /* check if read or write would be outside of device size or exceed buffer */
    if (unlikely((sector + (nsectors >> ECO32SDC_SECTOR_SHIFT)) > eco32sdc_dev.size)) {
        dev_err(eco32sdc_dev.dev, "request outside device size\n");
        __blk_mq_end_request(eco32sdc_dev.current_request, BLK_STS_IOERR);
        return BLK_STS_IOERR;
    }

    if (unlikely(nsectors > 8)) {
        dev_err(eco32sdc_dev.dev, "request exceeds per request sector limit\n");
        __blk_mq_end_request(eco32sdc_dev.current_request, BLK_STS_IOERR);
        return BLK_STS_IOERR;
    }

    /* no more requests till the current one is done */
    blk_mq_stop_hw_queues(eco32sdc_dev.queue);
    eco32sdc_dev.busy = 1;

    /* schedule tasklet to perform action on pending request */
    tasklet_schedule(&eco32sdc_tasklet);

    return BLK_STS_OK;
}

static const struct blk_mq_ops eco32sdc_mq_ops = {
    .queue_rq = eco32sdc_queue_rq,
};

/**************************************************************/

/*
 * Module setup and teardown
 */


static int eco32sdc_probe(struct platform_device* dev)
{

    int err;
    unsigned int base;
    unsigned int len;

    eco32sdc_dev.dev = &dev->dev;

    /* read device node and obtain needed properties */
    if (dev->dev.of_node) {
        struct device_node* np = dev->dev.of_node;

        err = of_property_read_u32_index(np, "reg", 0, &base);

        if (err)
            goto out_no_property;

        err = of_property_read_u32_index(np, "reg", 1, &len);

        if (err)
            goto out_no_property;

        eco32sdc_dev.base = (unsigned int*)(base | PAGE_OFFSET);

    } else {
        dev_err(&dev->dev, "device node not present\n");
        return -ENODEV;
    }

    if (!IS_MODULE(CONFIG_ECO32_SDC)) {
        if (eco32_device_probe((unsigned long)eco32sdc_dev.base)) {
            dev_err(&dev->dev, "device not present on the bus\n");
            return -ENODEV;
        }
    }

    /* register mem region */
    if (devm_request_mem_region(&dev->dev, base, len, ECO32SDC_DEV_NAME) == NULL) {
        dev_err(&dev->dev, "could not request mem region\n");
        return -EBUSY;
    }

    /* reset struct values */
    eco32sdc_dev.lastSent = 0;
    eco32sdc_dev.busy = 0;
    eco32sdc_dev.current_request = NULL;

    /* init sd card */
    if (!initSDC()) {
        dev_err(&dev->dev, "could not initialize sdcard\n");
        return -EBUSY;
    }

    /* check if sdcard slot is empty */
    if (dskcapsdc() == 1024) {
        dev_info(&dev->dev, "no sdcard in sdcard slot");
        return -ENODEV;
    }

    /* register block device */
    if ((err = register_blkdev(ECO32SDC_MAJOR, ECO32SDC_DEV_NAME)) < 0) {
        dev_err(&dev->dev, "could not register block device\n");
        return err;
    }

    /* allocate gendisk struct */
    if ((eco32sdc_dev.gd = alloc_disk(ECO32SDC_MINORS)) == NULL) {
        dev_err(&dev->dev, "could not allocate gendisk\n");
        err = -ENOMEM;
        goto out_unregister;
    }

    /* init request queue */
    eco32sdc_dev.queue = blk_mq_init_sq_queue(&eco32sdc_dev.tag_set, &eco32sdc_mq_ops, 1, BLK_MQ_F_SHOULD_MERGE);
    if (eco32sdc_dev.queue == NULL) {
        dev_err(&dev->dev, "could not initialize request queue\n");
        err = -ENOMEM;
        goto out_free;
    }

    /*
     * set gendisk up
     * add it to black layer as last step because the kernel will
     * instantly want to read from the disk but we are not ready yet
     */
    eco32sdc_dev.gd->major = ECO32SDC_MAJOR;
    eco32sdc_dev.gd->first_minor = 1;
    eco32sdc_dev.gd->queue = eco32sdc_dev.queue;
    eco32sdc_dev.gd->fops = &eco32sdc_ops;
    strcpy(eco32sdc_dev.gd->disk_name, ECO32SDC_DEV_NAME);
    eco32sdc_dev.size = dskcapsdc();
    set_capacity(eco32sdc_dev.gd, eco32sdc_dev.size);

    /*
     * let the block layer know how our hardware works and how much at a
     * time we can handle
     *
     * 8 is the minimum. we should not to more since we need to poll the data
     * which can take some time
     */
    blk_queue_max_hw_sectors(eco32sdc_dev.queue, 8);
    blk_queue_logical_block_size(eco32sdc_dev.queue, ECO32SDC_SECTOR_SIZE);

    /* finally add disk */
    add_disk(eco32sdc_dev.gd);

    return 0;
out_no_property:
    dev_err(&dev->dev, "could not read all necessary properties from device tree\n");
    return -ENODEV;
out_free:
    del_gendisk(eco32sdc_dev.gd);
out_unregister:
    unregister_blkdev(ECO32SDC_MAJOR, ECO32SDC_DEV_NAME);
    return err;
}


static int eco32sdc_remove(struct platform_device* dev)
{
    unregister_blkdev(ECO32SDC_MAJOR, ECO32SDC_DEV_NAME);
    del_gendisk(eco32sdc_dev.gd);
    blk_cleanup_queue(eco32sdc_dev.queue);
    return 0;
}


static struct of_device_id eco32sdc_of_ids[] = {
    {
        .compatible = "thm,eco32-sdc",
    },
    { },
};

static struct platform_driver eco32sdc_platform_driver = {
    .probe = eco32sdc_probe,
    .remove = eco32sdc_remove,
    .driver = {
        .name = ECO32SDC_DRV_NAME,
        .of_match_table = of_match_ptr(eco32sdc_of_ids),
    },
};
module_platform_driver(eco32sdc_platform_driver);

MODULE_LICENSE("GPL");
