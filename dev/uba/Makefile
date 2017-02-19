CC = gcc
LD = gcc
CFLAGS = -g -O3 -c -DDEBUG
LDFLAGS = -g
INCLUDES = -I../.. -I.
LIBS = 

BINDIR = .
LIBUBA = ${BINDIR}/libuba.a

OBJS = \
	bdv.o \
	dhu.o \
	dl.o \
	dz.o \
	kwl.o \
	mrv.o \
	lp.o \
	lp20.o \
	rh.o \
	rl.o \
	rq.o \
	tcu.o \
	tq.o \
	ts.o \
	uba.o \
	xq.o 

all: ${LIBUBA}

${LIBUBA}: ${OBJS}
	${AR} rc $@ ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -rf ${LIBUBA} *.o *.il
