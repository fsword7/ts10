CC = gcc
LD = gcc
CFLAGS = -g -O3 -c 
LDFLAGS = -g
INCLUDES = -I../..
LIBS = 

BINDIR = .
LIBMBA = ${BINDIR}/libmba.a

OBJS = \
	mba.o \
	rp.o \
	tu.o

all: ${LIBMBA}

${LIBMBA}: ${OBJS}
	${AR} rc $@ ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -rf ${LIBMBA} *.o *.il
