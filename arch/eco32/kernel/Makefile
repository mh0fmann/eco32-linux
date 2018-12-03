#
# Makefile for the ECO32-specific parts of the kernel.
#

extra-y := head.o vmlinux.lds

obj-y += syscalls.o irq.o traps.o setup.o pm.o
obj-y += process.o ptrace.o archlocalirq.o
obj-y += die.o entry.o signal.o switch.o
obj-y += topology.o time.o restart.o

obj-$(CONFIG_PROC_FS) += proc.o
obj-$(CONFIG_EARLY_PRINTK) += early_printk.o
