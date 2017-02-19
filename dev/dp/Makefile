CC = gcc
LD = gcc
CFLAGS = -g -O3 -c 
LDFLAGS = -g
INCLUDES = -I../..
LIBS = 

OBJS = \
dp_main.o

all: ${OBJS}

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} $<

clean:
	@rm -rf *.o *.il
