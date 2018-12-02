#
# arch/eco32/boot/Makefile
#

# uImage build relies on mkimage being available on your host for eco32 traget
# You will need to build barebox for ECO32, rename mkimage to eco32-unkown-linux-gnu-mkimage
# and make sure it's reacable from your PATH

clean-files := vmlinux.bin uImage

OBJCOPYFLAGS_vmlinux.bin := -O binary

UIMAGE_LOADADDR = 0xC0000000
UIMAGE_ENTRYADDR = 0xC0000000

$(obj)/vmlinux.bin: vmlinux FORCE
	$(call if_changed,objcopy)

$(obj)/uImage: $(obj)/vmlinux.bin FORCE
	$(call if_changed,uimage)
