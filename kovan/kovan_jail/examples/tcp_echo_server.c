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
	
	char buf[LINE_MAX];
	char *ret;
	FILE * client = argument->proto.tcpArg.client;

	printf("Starting Echo Server...\n");

	while( (ret = gs_read_line(client, buf, 1)) != NULL )
	{
		fwrite (buf , 1 , strlen(buf) , client);
		fwrite ("\n" , 1 , strlen("\n") , client);
			
		printf("compare: '%s', quit\n", buf);
		if( strncmp( buf, "quit", strlen(buf)) == 0)
			break;
	}

	printf("Echo Server finished\n");

	return 1;

}


int main(void){

	gs_start( TCP, "tcp_echo_pid", "192.168.56.101", "80", echo, preprocessor, postprocess);

	return 1;
}
