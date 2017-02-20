CC = gcc
LD = gcc
CFLAGS = -g -O3 -c -DDEBUG
LDFLAGS = -g
INCLUDES = -I../..
LIBS = 

OBJS = \
	dte20.o \
	kxio.o \
	rh20.o

all: ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -f *.o *.il
