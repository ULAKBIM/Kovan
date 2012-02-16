COMPILER=gcc
CCFLAGS=-g  

all: kovan_dns

kovan_dns: kovan_dns.c ../shared/generic_server.o 
	${COMPILER} ${CCFLAGS} -o kovan_dns kovan_dns.c ../shared/generic_server.o -Wall 

clean:
	rm -f kovan_dns


