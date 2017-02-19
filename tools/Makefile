CC = gcc
LD = gcc
CFLAGS = -g -O3 -c 
LDFLAGS = -g
INCLUDES = -I.. -I.
LIBS = 

DUMP_OBJS = dump.o ../emu/vtape.o
DUMP_OBJS2 = dump.o vtape.o

dump: ${DUMP_OBJS}
	${CC} ${LDFLAGS} -o $@ ${DUMP_OBJS2} ${LIBS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -f *.o
	@rm -f dump
