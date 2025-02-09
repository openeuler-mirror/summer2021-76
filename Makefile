C      = gcc
CFLAGS  = -Wall -O0 -g -W
ALL_CFLAGS = $(CFLAGS) -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

PROGS = ioheat

ALL = $(PROGS)

$(PROGS): | depend

all: $(ALL)

%.o: %.c
	$(CC) -o $*.o -c $(ALL_CFLAGS) $<

ioheat: main.o tracers.o rbtree.o blkparse.o blkparse_fmt.o # blktrace.o
	$(CC) $(ALL_CFLAGS) -o $@ $(filter %.o,$^) -lm

depend:
	@$(CC) -MM $(ALL_CFLAGS) *.c 1> .depend

clean:
	-rm -f *.o $(PROGS) .depend

ifneq ($(wildcard .depend),)
include .depend
endif