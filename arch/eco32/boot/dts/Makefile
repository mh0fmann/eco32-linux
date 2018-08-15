#
# Makefile for device tree blob
#

ifneq '$(CONFIG_ECO32_BUILTIN_DTB)' '""'
BUILTIN_DTB := $(patsubst "%",%,$(CONFIG_ECO32_BUILTIN_DTB)).dtb.o
else
BUILTIN_DTB :=
endif
obj-y += $(BUILTIN_DTB)

clean-files := *.dtb.S
