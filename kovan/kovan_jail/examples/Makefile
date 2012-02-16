COMPILER=gcc
CCFLAGS=-g  

all: udp_echo_server tcp_echo_server

udp_echo_server: udp_echo_server.c ../shared/generic_server.o 
	${COMPILER} ${CCFLAGS} -o udp_echo_server udp_echo_server.c ../shared/generic_server.o -Wall 

tcp_echo_server: tcp_echo_server.c ../shared/generic_server.o 
	${COMPILER} ${CCFLAGS} -o tcp_echo_server tcp_echo_server.c ../shared/generic_server.o -Wall 

clean:
	rm -f udp_echo_server
	rm -f tcp_echo_server

