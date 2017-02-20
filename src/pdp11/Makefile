CC = gcc
LD = gcc
CFLAGS = -g -O3 -c -DDEBUG
LDFLAGS = -g
INCLUDES = -I..
LIBS = 

BINDIR   = .
LIBP11 = ${BINDIR}/libp11.a

OBJS = \
	commands.o \
	cpu_eis.o \
	cpu_main.o \
	disasm.o \
	inst.o \
	kdf11_sys.o \
	kdj11_sys.o \
	memory.o \
	system.o \
	uqba.o 

all: ${LIBP11}

${LIBP11}: ${OBJS}
	${AR} rc $@ ${OBJS}

vax:
	${LD} ${LDFLAGS} -o $@ ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -f ${LIBP11} *.o *.il
