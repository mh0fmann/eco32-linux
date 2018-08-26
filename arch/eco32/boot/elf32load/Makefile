#
# Makefile for ECO32 object file loader
#

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -g
LDLIBS = -lm

SRCS = loadelf.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = elf32load

.PHONY:		all clean

all:		$(BIN)

$(BIN):		$(OBJS)
		$(CC) $(LDFLAGS) -o $(BIN) $(OBJS) $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

clean:
		$(Q)echo "  CLEAN   arch/eco32/boot/elf32load"
		$(Q)rm -f *~ $(OBJS) $(BIN)
