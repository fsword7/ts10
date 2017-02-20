CC = gcc
LD = gcc
CFLAGS = -g -O3 -c 
LDFLAGS = -g
INCLUDES = -I.. -I.
LIBS = 

DUMP_OBJS = dump10.o ../emu/vtape.o
DUMP_OBJS2 = dump10.o vtape.o

dump10: ${DUMP_OBJS}
	${CC} ${LDFLAGS} -o $@ ${DUMP_OBJS2} ${LIBS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -f *.o dump10 dump10.exe
