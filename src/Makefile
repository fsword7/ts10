CC = gcc
LD = gcc
CFLAGS = -g -O3 -c -DHAVE_SIGACTION -DDEBUG
#CFLAGS = -g -O3 -c
#CFLAGS = -g -pg -O3 -c
P10FLAGS = -DOPT_XADR #-DIDLE
LDFLAGS = -g
INCLUDES = -I.
LIBS = 

LIBTS10  = libts10.a
LIBMBA   = libmba.a
LIBUBA   = libuba.a
LIBP10   = libp10.a
LIBP11   = libp11.a
LIBVAX   = libvax.a

TS10_LIBS = ${LIBTS10} ${LIBA2} ${LIBP10} ${LIBP11} \
	${LIBVAX} ${LIBUBA} ${LIBMBA} ${LIBTS10}

all: ts10

ts10:
	cd emu; make CC="${CC}" LD="${LD}" CFLAGS="${CFLAGS}" BINDIR=".." all
	cd dev/mba; make CC="${CC}" LD="${LD}" CFLAGS="${CFLAGS}" BINDIR="../.." all
	cd dev/uba; make CC="${CC}" LD="${LD}" CFLAGS="${CFLAGS}" BINDIR="../.." all
	cd pdp10; make CC="${CC}" LD="${LD}" CFLAGS="${CFLAGS} ${P10FLAGS}" BINDIR=".." all
	cd pdp11; make CC="${CC}" LD="${LD}" CFLAGS="${CFLAGS}" BINDIR=".." all
	cd vax; make CC="${CC}" LD="${LD}" CFLAGS="${CFLAGS}" BINDIR=".." all
	${CC} ${LDFLAGS} -o $@ ${TS10_LIBS} ${LIBS}

dist:
	cd ..; tar -czvf ts10-`date +%y%m%d`.tgz ts10

clean:
	cd emu; make clean
	cd dev/mba; make clean
	cd dev/uba; make clean
	cd pdp10; make clean
	cd pdp11; make clean
	cd vax; make clean
	@rm -f ts10 ts10.exe ${TS10_LIBS}
