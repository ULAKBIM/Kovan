COMPILER=gcc
CCFLAGS=-g  

all: kovan_http

kovan_http: kovan_http.c ../shared/generic_server.o 
	${COMPILER} ${CCFLAGS} -o kovan_http kovan_http.c ../shared/generic_server.o -Wall 

clean:
	rm -f kovan_http
