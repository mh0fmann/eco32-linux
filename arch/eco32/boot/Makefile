#
# arch/eco32/boot/Makefile
#

vmlinux.bin:
	$(Q)$(MAKE) -C elf32load

vmlinuxboot.img:
	$(Q)$(MAKE) -C bootloader

archclean:
	$(Q)$(MAKE) -C elf32load clean
	$(Q)$(MAKE) -C bootloader clean
