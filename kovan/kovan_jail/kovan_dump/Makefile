COMPILER=gcc
CCFLAGS=-g  

all: kovan_dump 

kovan_dump: kovan_dump.c 
	${COMPILER} ${CCFLAGS} -o kovan_dump -lpcap kovan_dump.c -Wall 

clean:
	rm -f kovan_dump 


