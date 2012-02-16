COMPILER=gcc
CCFLAGS=-g  

all: kovan_smtp

kovan_smtp: kovan_smtp.c ../shared/generic_server.o 
	${COMPILER} ${CCFLAGS} -o kovan_smtp kovan_smtp.c ../shared/generic_server.o -Wall 

clean:
	rm -f kovan_smtp
