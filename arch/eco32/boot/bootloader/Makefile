#
# Makefile to build the partition boot record
#

.PHONY:		all clean

all:		pbr.bin mvstrt.bin

pbr.bin:	pbr
		$(Q)../elf32load/elf32load -v pbr pbr.bin

pbr:		pbr.o
		$(Q)echo "  LD		arch/eco32/boot/bootloader/pbr"
		$(Q)$(LD) -static -nostdlib -T pbr.lds -o pbr pbr.o

pbr.o:		pbr.S
		$(Q)echo "  AS		arch/eco32/boot/bootloader/pbr.o"
		$(Q)$(AS) -o pbr.o pbr.S
		
mvstrt.bin:	mvstrt
		$(Q)../elf32load/elf32load -v mvstrt mvstrt.bin

mvstrt:		mvstrt.o
		$(Q)echo "  LD		arch/eco32/boot/bootloader/mvstrt"
		$(Q)$(LD) -T mvstrt.lds -o mvstrt mvstrt.o

mvstrt.o:	mvstrt.S
		$(Q)echo "  AS		arch/eco32/boot/bootloader/mvstrt.o"
		$(Q)$(AS) -o mvstrt.o mvstrt.S

clean:
		$(Q)echo "  CLEAN   arch/eco32/boot/bootloader"
		$(Q)rm -f *~ pbr.o pbr pbr.bin mvstrt.o mvstrt mvstrt.bin
