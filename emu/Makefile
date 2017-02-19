CC = gcc
LD = gcc
CFLAGS = -g -O3 -c -DDEBUG
LDFLAGS = -g
INCLUDES = -I..
LIBS = 

BINDIR  = .
LIBTS10 = ${BINDIR}/libts10.a

OBJS = \
	commands.o \
	debug.o \
	ether.o \
	help.o \
	main.o \
	misc.o \
	panel.o \
	socket.o \
	system.o \
	timer.o \
	vdisk.o \
	vtape.o

all: ${LIBTS10}

${LIBTS10}: ${OBJS}
	${AR} rc $@ ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -f ${LIBTS10} *.o *.il
