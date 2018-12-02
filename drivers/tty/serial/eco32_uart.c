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
 * eco32_uart.c -- the ECO32 serial line driver
 */


#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <asm/irq.h>
#include <asm/devprobe.h>


#define ECO32UART_DRV_NAME      "eco32uart"
#define ECO32UART_DEV_NAME      "ttyS"
#define ECO32UART_CONS_NAME     ECO32UART_DEV_NAME
#define ECO32UART_MAJOR         4
#define ECO32UART_MINOR         64
#define ECO32UART_NR_UARTS      2

#define ECO32UART_DFLT_BAUD     38400
#define ECO32UART_DFLT_BITS     '8'
#define ECO32UART_DFLT_PRTY     'n'
#define ECO32UART_DFLT_FLOW     'n'

#define ECO32UART_PORT_SIZE     (4 * 4)

#define ECO32UART_RCVR_CTRL     0
#define ECO32UART_RCVR_CTRL_RDY 0x0001
#define ECO32UART_RCVR_CTRL_IEN 0x0002

#define ECO32UART_RCVR_DATA     4

#define ECO32UART_XMTR_CTRL     8
#define ECO32UART_XMTR_CTRL_RDY 0x0001
#define ECO32UART_XMTR_CTRL_IEN 0x0002

#define ECO32UART_XMTR_DATA     12

#define ECO32UART_CLK           50000000

#define PORT_ECO32              234


/*
 * We wrap our port struct around the generic one
 */
struct uart_eco32_port {
    struct uart_port port;
    struct device* dev;
    int tx_irq;
    int rx_irq;
    char dev_name[10];
};


/**************************************************************/

/*
 * UART register read/write
 */


static inline u32 eco32uart_in(u32 offset, struct uart_port* port)
{
    return ioread32be(port->membase + offset);
}


static inline void eco32uart_out(u32 val, u32 offset, struct uart_port* port)
{
    iowrite32be(val, port->membase + offset);
}


/**************************************************************/

/*
 * UART interrupt handlers
 */


static irqreturn_t eco32uart_rx_interrupt_handler(int irq, void* dev)
{
    struct uart_port* port;
    struct tty_port* tport;
    unsigned int ctrl, ch;
    unsigned int flag;
    int max_count;

    port = (struct uart_port*)dev;
    tport = &port->state->port;
    max_count = 256;
    ctrl = eco32uart_in(ECO32UART_RCVR_CTRL, port);

    while ((ctrl & ECO32UART_RCVR_CTRL_RDY) && max_count--) {
        ch = eco32uart_in(ECO32UART_RCVR_DATA, port);
        flag = TTY_NORMAL;
        port->icount.rx++;
        tty_insert_flip_char(tport, ch, flag);
        ctrl = eco32uart_in(ECO32UART_RCVR_CTRL, port);
    }

    tty_flip_buffer_push(tport);

    return IRQ_HANDLED;
}

static void eco32uart_stop_tx(struct uart_port* port);
static irqreturn_t eco32uart_tx_interrupt_handler(int irq, void* dev)
{
    struct uart_port* port;
    struct circ_buf* xmit;
    int count;

    port = (struct uart_port*)dev;

    xmit = &port->state->xmit;

    if (port->x_char) {
        eco32uart_out(port->x_char, ECO32UART_XMTR_DATA, port);
        port->icount.tx++;
        port->x_char = 0;
        return IRQ_HANDLED;
    }

    if (uart_tx_stopped(port) || uart_circ_empty(xmit)) {
        eco32uart_stop_tx(port);
        return IRQ_HANDLED;
    }

    count = port->fifosize;

    while (!uart_circ_empty(xmit) &&(count-- > 0)) {
        eco32uart_out(xmit->buf[xmit->tail], ECO32UART_XMTR_DATA, port);
        xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
        port->icount.tx++;
    }

    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS) {
        uart_write_wakeup(port);
    }

    if (uart_circ_empty(xmit)) {
        eco32uart_stop_tx(port);
    }

    return IRQ_HANDLED;
}

/**************************************************************/

/*
 * UART operations
 */


static unsigned int eco32uart_tx_empty(struct uart_port* port)
{
    unsigned int ctrl;

    ctrl = eco32uart_in(ECO32UART_XMTR_CTRL, port);
    return ctrl & ECO32UART_XMTR_CTRL_RDY ? TIOCSER_TEMT : 0;
}


static void eco32uart_set_mctrl(struct uart_port* port, unsigned int mctrl)
{
    /* no modem control */
}


static unsigned int eco32uart_get_mctrl(struct uart_port* port)
{
    /* no modem control */
    return TIOCM_CTS | TIOCM_DSR | TIOCM_CAR;
}


static void eco32uart_stop_tx(struct uart_port* port)
{

    /* disable interrupts for transmitter */
    eco32uart_out(0, ECO32UART_XMTR_CTRL, port);
}


static void eco32uart_start_tx(struct uart_port* port)
{

    /* enable interrupts for transmitter */
    eco32uart_out(ECO32UART_XMTR_CTRL_IEN, ECO32UART_XMTR_CTRL, port);
}


static void eco32uart_stop_rx(struct uart_port* port)
{

    /* disable interrupts for reciever */
    eco32uart_out(0, ECO32UART_RCVR_CTRL, port);
}


static void eco32uart_break_ctl(struct uart_port* port, int ctl)
{
    /* no break control */
}


static int eco32uart_startup(struct uart_port* port)
{

    struct uart_eco32_port* eco32port = container_of(port, struct uart_eco32_port, port);

    /* request irq and set isr */
    if (request_irq(eco32port->tx_irq, eco32uart_tx_interrupt_handler, 0, eco32port->dev_name, port)) {
        dev_err(eco32port->dev, "could not request irq for eco32uart\n");
        return -EIO;
    }

    if (request_irq(eco32port->rx_irq, eco32uart_rx_interrupt_handler, 0, eco32port->dev_name, port)) {
        dev_err(eco32port->dev, "could not request irq for eco32uart\n");
        return -EIO;
    }

    set_ISR(eco32port->tx_irq, do_IRQ);
    set_ISR(eco32port->rx_irq, do_IRQ);

    /* disable interrupts for transmitter */
    eco32uart_stop_tx(port);

    /* enable interrupts for receiver */
    eco32uart_out(ECO32UART_RCVR_CTRL_IEN, ECO32UART_RCVR_CTRL, port);

    return 0;
}


static void eco32uart_shutdown(struct uart_port* port)
{

    struct uart_eco32_port* eco32port = container_of(port, struct uart_eco32_port, port);

    /* disable interrupts for transmitter */
    eco32uart_stop_tx(port);

    /* disable interrupts for receiver */
    eco32uart_out(0, ECO32UART_RCVR_CTRL, port);

    /* free irq and reset isr */
    free_irq(eco32port->tx_irq, port);
    free_irq(eco32port->rx_irq, port);
    set_ISR(eco32port->tx_irq, def_xcpt_handler);
    set_ISR(eco32port->rx_irq, def_xcpt_handler);
}


static void eco32uart_set_termios(struct uart_port* port, struct ktermios* termios, struct ktermios* old)
{
    unsigned int baud, ccpb;
    unsigned long flags;

    baud = uart_get_baud_rate(port, termios, old, 50, port->uartclk / 16);
    ccpb = port->uartclk / baud;
    //pr_info("baud = 0x%08X, ccpb = 0x%08X\n", baud, ccpb);
    spin_lock_irqsave(&port->lock, flags);
    /* number of data bits: not configurable */
    /* number of stop bits: not configurable */
    /* parity: not configurable */
    /* FIFO size: not configurable */
    uart_update_timeout(port, termios->c_cflag, baud);
    port->read_status_mask = 0;
    port->ignore_status_mask = 0;
    /* baud rate: not configurable */
    spin_unlock_irqrestore(&port->lock, flags);
}


static const char* eco32uart_type(struct uart_port* port)
{
    return port->type == PORT_ECO32 ? ECO32UART_DRV_NAME : NULL;
}


static void eco32uart_release_port(struct uart_port* port)
{
    release_mem_region(port->mapbase, ECO32UART_PORT_SIZE);
}


static int eco32uart_request_port(struct uart_port* port)
{
    return request_mem_region(port->mapbase, ECO32UART_PORT_SIZE, ECO32UART_DRV_NAME) != NULL ? 0 : -EBUSY;
}


static void eco32uart_config_port(struct uart_port* port, int flags)
{
    if (flags & UART_CONFIG_TYPE) {
        port->type = PORT_ECO32;
        eco32uart_request_port(port);
    }
}


static int eco32uart_verify_port(struct uart_port* port, struct serial_struct* ser)
{
    /* we don't want the core code to modify any port params */
    return -EINVAL;
}


static const struct uart_ops eco32uart_ops = {
    .tx_empty = eco32uart_tx_empty,
    .set_mctrl = eco32uart_set_mctrl,
    .get_mctrl = eco32uart_get_mctrl,
    .stop_tx = eco32uart_stop_tx,
    .start_tx = eco32uart_start_tx,
    .stop_rx = eco32uart_stop_rx,
    .break_ctl = eco32uart_break_ctl,
    .startup = eco32uart_startup,
    .shutdown = eco32uart_shutdown,
    .set_termios = eco32uart_set_termios,
    .type = eco32uart_type,
    .release_port = eco32uart_release_port,
    .request_port = eco32uart_request_port,
    .config_port = eco32uart_config_port,
    .verify_port = eco32uart_verify_port,
};


/**************************************************************/

/*
 * Module setup and teardown
 */

#ifdef CONFIG_SERIAL_ECO32_CONSOLE
static struct console eco32uart_console;
static struct uart_port* eco32uart_console_port = NULL;
#endif /* CONFIG_SERIAL_ECO32_CONSOLE */

static int isRegistred = 0;
static struct uart_driver eco32uart_uart_driver = {
    .owner = THIS_MODULE,
    .driver_name = ECO32UART_DRV_NAME,
    .dev_name = ECO32UART_DEV_NAME,
    .major = ECO32UART_MAJOR,
    .minor = ECO32UART_MINOR,
    .nr = ECO32UART_NR_UARTS,
#ifdef CONFIG_SERIAL_ECO32_CONSOLE
    .cons = &eco32uart_console,
#endif /* CONFIG_SERIAL_ECO32_CONSOLE */
};


static int eco32uart_probe(struct platform_device* dev)
{

    struct uart_eco32_port* port;
    struct device_node* node;
    unsigned int base, clk, tx_irq, rx_irq;
    int err;
    static int line = 0;

    /* try to register the uart driver befor doing anything else */
    if (!isRegistred) {
        if ((err = uart_register_driver(&eco32uart_uart_driver)) != 0) {
            dev_err(&dev->dev, "could not register uart driver");
            return -ENOMEM;
        }
        isRegistred = 1;
    }

    /* read device node and obtain needed properties */
    node = dev->dev.of_node;

    base = of_iomap(node, 0);
    if (!base) goto out_no_property;

    tx_irq = irq_of_parse_and_map(node, 0);
    if (!tx_irq) goto out_no_property;

    rx_irq = irq_of_parse_and_map(node, 1);
    if (!rx_irq) goto out_no_property;

    err = of_property_read_u32(node, "clock-frequency", &clk);
    if (err) goto out_no_property;

    if (eco32_device_probe((unsigned long)(base | PAGE_OFFSET))) {
        dev_err(&dev->dev, "device not present on the bus\n");
        return -ENODEV;
    }

    /* allocate port and fill it */
    port = devm_kzalloc(&dev->dev, sizeof(struct uart_eco32_port), GFP_KERNEL);

    if (!port) {
        dev_err(&dev->dev, "could not allocate port\n");
        return -ENOMEM;
    }

    port->dev = &dev->dev;
    port->port.membase = (unsigned char __iomem*)(base);
    port->port.mapbase = __pa(base);
    port->port.iotype = SERIAL_IO_MEM;
    port->port.uartclk = clk;
    port->port.fifosize = 1;
    port->port.ops = &eco32uart_ops;
    port->port.flags = UPF_BOOT_AUTOCONF;
    port->port.line = line;
    port->tx_irq = tx_irq;
    port->rx_irq = rx_irq;
    sprintf(port->dev_name, "ttySECO%d", line++);

    /* set port as driver data for access on unregister */
    platform_set_drvdata(dev, port);

#ifdef CONFIG_SERIAL_ECO32_CONSOLE
    if (eco32uart_console_port == NULL) {
        eco32uart_console_port = &port->port;
    }
#endif /* CONFIG_ECO32UART_CONSOLE */

    /* finally add port */
    err = uart_add_one_port(&eco32uart_uart_driver, &port->port);

    if (err) {
        dev_err(&dev->dev, "could not register port\n");
        return -EIO;
    }

    return 0;
out_no_property:
    dev_err(&dev->dev, "could not read all necessary properties from device tree\n");
    return -ENODEV;
}


static int eco32uart_remove(struct platform_device* dev)
{
    struct uart_eco32_port* port = platform_get_drvdata(dev);
    uart_remove_one_port(&eco32uart_uart_driver, &port->port);
    return 0;
}


static struct of_device_id eco32uart_of_ids[] = {
    {
        .compatible = "thm,eco32-uart",
    },
    {0}
};


static struct platform_driver eco32uart_platform_driver = {
    .probe = eco32uart_probe,
    .remove = eco32uart_remove,
    .driver = {
        .name = ECO32UART_DRV_NAME,
        .of_match_table = of_match_ptr(eco32uart_of_ids),
    },
};
module_platform_driver(eco32uart_platform_driver);


MODULE_LICENSE("GPL");



/**************************************************************/

/*
 * Console driver
 */

#ifdef CONFIG_SERIAL_ECO32_CONSOLE

static void eco32uart_console_putchar(struct uart_port* port, int ch)
{
    unsigned long ctrl;
    unsigned long timeout = jiffies + msecs_to_jiffies(1000);

    while (1) {
        ctrl = eco32uart_in(ECO32UART_XMTR_CTRL, port);

        if (ctrl & ECO32UART_XMTR_CTRL_RDY){
            break;
        }

        if (time_after(jiffies, timeout)) {
            dev_warn(port->dev, "timout wating for TX buffer empty\n");
        }

        cpu_relax();
    }

    eco32uart_out(ch, ECO32UART_XMTR_DATA, port);
}


static void eco32uart_console_write(struct console* co, const char* s, unsigned int count)
{
    unsigned long flags;
    int locked = 1;
    struct uart_port* port = eco32uart_console_port;

    if (oops_in_progress) {
        locked = spin_trylock_irqsave(&port->lock, flags);
    } else {
        spin_lock_irqsave(&port->lock, flags);
    }

    /* use helper function to write the console string */
    uart_console_write(port, s, count, eco32uart_console_putchar);

    if (locked) {
        spin_unlock_irqrestore(&port->lock, flags);
    }
}


static int eco32uart_console_setup(struct console* co, char* options)
{

    struct uart_port* port;
    int baud = ECO32UART_DFLT_BAUD;
    int bits = ECO32UART_DFLT_BITS;
    int prty = ECO32UART_DFLT_PRTY;
    int flow = ECO32UART_DFLT_FLOW;

    if (co->index < 0 || co->index >= ECO32UART_NR_UARTS) {
        return -EINVAL;
    }

    port = eco32uart_console_port;

    if (options) {
        uart_parse_options(options, &baud, &prty, &bits, &flow);
    }

    return uart_set_options(port, co, baud, prty, bits, flow);
}


static struct console eco32uart_console = {
    .name = ECO32UART_CONS_NAME,
    .write = eco32uart_console_write,
    .device = uart_console_device,
    .setup = eco32uart_console_setup,
    .flags = CON_PRINTBUFFER,
    .index = -1,
    .data = &eco32uart_uart_driver
};

#endif /* CONFIG_ECO32UART_CONSOLE */
