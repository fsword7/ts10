CC = gcc
LD = gcc
CFLAGS = -g -O3 -c -DDEBUG
LDFLAGS = -g
INCLUDES = -I..
LIBS = 

BINDIR = .
LIBVAX = ${BINDIR}/libvax.a

OBJS = \
	commands.o \
	cpu_branch.o \
	cpu_compare.o \
	cpu_convert.o \
	cpu_field.o \
	cpu_fpa.o \
	cpu_integer.o \
	cpu_intexc.o \
	cpu_main.o \
	cpu_misc.o \
	cpu_mmu.o \
	cpu_move.o \
	cpu_queue.o \
	cpu_rcnt.o \
	cpu_stack.o \
	cpu_string.o \
	dev_cty.o \
	disasm.o \
	ka630_qba.o \
	ka630_sys.o \
	ka650_cmd.o \
	ka650_qba.o \
	ka650_sys.o \
	ka780_mba.o \
	ka780_sys.o \
	ka780_uba.o \
	inst.o \
	memory.o \
	system.o

all: ${LIBVAX}

${LIBVAX}: ${OBJS}
	${AR} rc $@ ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -f ${LIBVAX} *.o *.il
