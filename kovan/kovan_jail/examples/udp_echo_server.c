#include<stdio.h>
#include<limits.h>
#include<string.h>
#include "../shared/generic_server.h"

 void postprocess( char * data ){
        /* do nothing */
}

char * preprocessor(){
        /* do nothing */

        return NULL;
}


int echo( ProcArg * argument  ){
	
	printf("Starting UDP Echo Server...\n");

	
	if(sendto(argument->proto.udpArg.socket, argument->proto.udpArg.buffer, argument->proto.udpArg.length, 0, argument->proto.udpArg.socket_addr, argument->proto.udpArg.addr_len )==-1){
		perror("sendto");
	}

	printf("received: '%s', '%d'\n", gs_trim(argument->proto.udpArg.buffer), argument->proto.udpArg.length);

	printf("UDP Echo Server finished\n");

	return 1;

}


int main(void){

	gs_start( UDP, "udp_echo_pid", "192.168.56.101", "81", echo, preprocessor, postprocess);

	return 1;
}
