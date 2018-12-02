#
# Makefile for the ECO32-specific parts of the kernel.
#

extra-y := head.o vmlinux.lds
obj-y := syscalls.o irq.o traps.o setup.o pm.o \
         process.o ptrace.o archlocalirq.o \
         die.o entry.o signal.o switch.o \
         topology.o time.o \
         restart.o cpu.o

obj-$(CONFIG_EARLY_PRINTK) += early_printk.o
