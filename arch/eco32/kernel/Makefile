#
# Makefile for the ECO32-specific parts of the kernel.
#

extra-y := head.o vmlinux.lds

obj-y += die.o
obj-y += entry.o
obj-y += irq.o
obj-y += pm.o
obj-y += process.o
obj-y += ptrace.o
obj-y += setup.o
obj-y += signal.o
obj-y += switch.o
obj-y += syscall_table.o
obj-y += time.o
obj-y += traps.o

obj-$(CONFIG_SYSFS) += topology.o
obj-$(CONFIG_PROC_FS) += proc.o
obj-$(CONFIG_EARLY_PRINTK) += early_printk.o
