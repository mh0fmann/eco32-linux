#
# arch/eco32/Makefile
#

KBUILD_DEFCONFIG := eco32sim_defconfig

#
# CPU-dependent compiler/assembler options for optimization
#
cflags-y += -D__linux__ -ffreestanding

#
# Board-dependent options and extra files
#
KBUILD_AFLAGS += $(cflags-y)
KBUILD_CFLAGS += $(cflags-y)
KBUILD_AFLAGS_MODULE +=
KBUILD_CFLAGS_MODULE +=
LDFLAGS += --oformat elf32-eco32
LDFLAGS_vmlinux	+= -static -nostdlib

LIBGCC	:= $(shell $(CC) -print-libgcc-file-name)

head-y := arch/eco32/kernel/head.o
libs-y += arch/eco32/lib/
libs-y += $(LIBGCC)
core-y += arch/eco32/kernel/ arch/eco32/mm/

ifneq '$(CONFIG_ECO32_BUILTIN_DTB)' '""'
BUILTIN_DTB := y
else
BUILTIN_DTB := n
endif
core-$(BUILTIN_DTB) += arch/eco32/boot/dts/

boot := arch/eco32/boot


ARCH_TARGETS := vmlinux.bin uImage


$(ARCH_TARGETS): vmlinux
	$(Q)$(MAKE) $(build)=$(boot) $(boot)/$@

archclean:
	$(Q)$(MAKE) $(clean)=$(boot)
	$(Q)rm -f vmlinux


define archhelp
	echo '  vmlinux.bin          - Raw binary kernel image ($(boot))'
	echo '  uImage               - U-Boot Image ($(boot))'
endef
