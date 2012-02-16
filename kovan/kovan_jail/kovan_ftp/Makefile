COMPILER=gcc
CCFLAGS=-g  

all: kovan_ftp

kovan_ftp: kovan_ftp.c ../shared/generic_server.o 
	${COMPILER} ${CCFLAGS} -o kovan_ftp kovan_ftp.c ../shared/generic_server.o -Wall 

clean:
	rm -f kovan_ftp
