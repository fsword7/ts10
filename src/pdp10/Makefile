CC = gcc
LD = gcc
CFLAGS = -g -O3 -c -DDEBUG
LDFLAGS = -g
INCLUDES = -I..
LIBS = 

BINDIR = .
LIBP10 = ${BINDIR}/libp10.a

DEV_OBJS = \
	dev_dte20.o \
	dev_rh20.o  \
	dev_kxio.o

OBJS = \
	asm.o \
	commands.o \
	cpu_extend.o \
	cpu_float.o \
	cpu_integer.o \
	cpu_main.o \
	disasm.o \
	instruction.o \
	kl10_apr.o \
	kl10_cca.o \
	kl10_main.o \
	kl10_mtr.o \
	kl10_pi.o \
	kl10_pag.o \
	kl10_sys.o \
	ks10_apr.o \
	ks10_fe.o \
	ks10_io.o \
	ks10_pag.o \
	ks10_pi.o \
	ks10_sys.o \
	ks10_tim.o \
	ks10_uba.o \
	memory.o \
	symbols.o \
	system.o

all: ${LIBP10}

${LIBP10}: ${DEV_OBJS} ${OBJS}
	${AR} rc $@ ${DEV_OBJS} ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -f ${LIBP10} *.o *.il
