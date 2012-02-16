#include "generic_server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/stat.h>

#define BUFFER_1K 1024
#define LISTEN_QUEUE 64

int server_socket;
int server_port;
char * server_addr;

void sigCHLD(int unused)
{
    waitpid(-1, 0, WNOHANG);
}

void gs_close( int sig )
{
	int ret;

	waitpid(-1, NULL, 0);
 
	printf("Closing Server...");
	if ((ret = close(server_socket)) != 0)
		syslog( LOG_WARNING, "Could not close server_socket in signal handler: %m");
	
	(void) signal(SIGINT, SIG_DFL);
	
	exit(1);
}


void gs_deamonize( char * pidfile){

	FILE *file;
	int pid, sid;

        syslog(LOG_INFO, "starting the daemonizing process");
 
        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
        	syslog( LOG_ERR, "daemonizing process fork: %m");
        	exit(EXIT_FAILURE);
        }

        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }
 
        /* Change the file mode mask */
        umask(0);
 
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
        }
 
        /* Change the current working directory */
        if ((chdir("/")) < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
        }

        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

	file = fopen(pidfile,"a+");
	fprintf(file,"%d\n",getpid());
	fclose(file);

}

void gs_start( ServerType eServerType, char * pidfile, const char * ip_addr, const char * port, int (*processor)(ProcArg *), char * (*preprocessor)( int * ), void (*postprocessor)(char *) ){

    signal(SIGCHLD, sigCHLD);

	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *res0;
	int ret;	
	int iAddrReuse = 1;

	if( pidfile != NULL )
		gs_deamonize(pidfile);

	(void) signal( SIGINT, gs_close);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;

	/* Server socket must be either TCP or UDP */	
	if( eServerType == TCP )
		hints.ai_socktype = SOCK_STREAM;
	else
		hints.ai_socktype = SOCK_DGRAM;
		

	hints.ai_flags = AI_PASSIVE;
	ret = getaddrinfo(ip_addr, port, &hints, &res0);
	
	if(ret != 0){
        	syslog( LOG_ERR, "Geaddrinfo couldn't get host information, error: %d", ret);
		return;
	}

	/* Iterate over all information objects to get a suitable one */	
	for (res = res0; res!= NULL ; res = res->ai_next)
	{
		server_socket = socket(res->ai_family, res->ai_socktype,
				res->ai_protocol);
		
		if (server_socket < 0) 
			continue;
		
		if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&iAddrReuse, sizeof(iAddrReuse)) == (-1)){
        		syslog( LOG_ERR, "setsockopt %m");
			return;
		}
		
		if (bind(server_socket, res->ai_addr, res->ai_addrlen) == 0) 
			break;

		if ((ret = close(server_socket)) != 0)
        		syslog( LOG_WARNING, "Could not close server_socket in parent: %m");

	}

	if( res == NULL )
	{
        	syslog( LOG_ERR, "Could not bind to any host");
		return;
	}
	
	if( res->ai_addr->sa_family == AF_INET )
	{
		server_addr = (char *) malloc(INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(((struct sockaddr_in *) res->ai_addr)->sin_addr), server_addr, INET_ADDRSTRLEN);


	}
	else if( res->ai_addr->sa_family == AF_INET6 )
	{
		server_addr = (char *) malloc(INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) res->ai_addr)->sin6_addr), server_addr, INET6_ADDRSTRLEN);

	}

	server_port = atoi(port);

	freeaddrinfo(res0);

	if( eServerType == TCP )
		gs_tcp( server_socket, processor, preprocessor, postprocessor );
	else if( eServerType == UDP )
		gs_udp( server_socket, processor, preprocessor, postprocessor );
	else
        	syslog( LOG_ERR, "Undefined Server Type %d", eServerType);

	free( server_addr);
	if ((ret = close(server_socket)) != 0)
        	syslog( LOG_WARNING, "Could not close server_socket %m");
        
	syslog( LOG_INFO, "Server closed...");

}

void gs_tcp( int socket, int (*processor)(ProcArg *), char * (*preprocessor)(int *), void (*postprocessor)( char *) ){

	int ret;
	int connfd;
	socklen_t addr_len;
	FILE * client = NULL;
	struct sockaddr  client_sock_addr;
	ProcArg argument;
	int client_port;
	char * client_addr;
	int preprocess_ret;
	
	argument.process_data = preprocessor( &preprocess_ret);

	if( preprocess_ret != 0 )
	{
		syslog( LOG_WARNING, "Preprocess failed");
		return;
	}
	ret = listen(server_socket, LISTEN_QUEUE);

	if( ret != 0 )
	{
		syslog( LOG_ERR, "TCP Socket listen error: %m");
		return;
	}
	
	while( 1 )
	{	
    		waitpid(-1, 0, WNOHANG);
		//addr_len = sizeof(client_sock_addr);
		addr_len = 32;

		/* accept remote connection */
		if((connfd=accept(server_socket, (struct sockaddr *)&client_sock_addr, &addr_len ))<0)
		{	
			syslog( LOG_ERR, "Socket accept failure in parent: %m");
			continue;
		}
		
		if( client_sock_addr.sa_family == AF_INET )
		{
			client_addr = (char *) malloc(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(((struct sockaddr_in *) &client_sock_addr)->sin_addr), client_addr, INET_ADDRSTRLEN);
			client_port =  ((struct sockaddr_in *) &client_sock_addr)->sin_port;
		}
		else if( client_sock_addr.sa_family == AF_INET6 )
		{
			client_addr = (char *) malloc(INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) &client_sock_addr)->sin6_addr), client_addr, INET6_ADDRSTRLEN);
			client_port =  ((struct sockaddr_in6 *) &client_sock_addr)->sin6_port;
		}
		else
		{
			syslog( LOG_ERR, "undefined sa_family");
			break;
		}
	
		/* fork() returns 0 to the child process only  */
		if ( (ret=fork()) == 0) {
		
			/* Close parent's socket, not needed in child */
			if ((ret = close(server_socket)) != 0)
				syslog( LOG_WARNING, "Undefined sa_family");

			/* Convert connection socket to file descriptor for easy IO */
			if ((client = fdopen(connfd, "r+")) == NULL)
			{
				syslog( LOG_ERR, "fdopen I/O socket: %m");
				return;
			}

			argument.server_addr = server_addr;
			argument.server_port = server_port;
			argument.client_addr = client_addr;
			argument.client_port = client_port;
			argument.proto.tcpArg.client = client;

			(*processor)(&argument); 

			/* Close the private communication socket and exit child process */
			if ((ret = fclose(client)) != 0)
				syslog( LOG_WARNING, "Could not close connection socket filedes in child process: %m");

			free(client_addr);

			/* Succesful termination of child process */
			exit(0);
		}

		free(client_addr);

		if( ret == -1 )
			syslog( LOG_ERR, "Fork problem in parent: %m");

		/* Close the private communication socket, which is not needed by the parent process any more */
		if ((ret = close(connfd)) != 0)
			syslog( LOG_WARNING, "Could not close connection socket parent process: %m");
	}
	postprocessor( argument.process_data );
}


void gs_udp( int socket, int (*processor)(ProcArg *), char * (*preprocessor)(int *), void (*postprocessor)(char *)  ){

	socklen_t addr_len;
	ProcArg argument;
	char buffer[BUFFER_1K];
	char client_addr_buffer[32]; /* a work around */
	int numbytes;
	char * client_addr;
	int client_port;
	int preprocess_ret;

	argument.process_data = preprocessor( &preprocess_ret );
	
	if( preprocess_ret != 0 )
	{
		syslog( LOG_ERR, "Preprocess failed");
		return;
	}
	
	while( 1 )
	{
		memset( buffer,0, BUFFER_1K);

		syslog( LOG_INFO, "UDP Server is waiting packets...");
		addr_len = 32;

		if( (numbytes = recvfrom(server_socket, buffer, BUFFER_1K, 0, (struct sockaddr *) client_addr_buffer, &addr_len)) ==-1 )
		{
			syslog( LOG_ERR, "recvfrom: %m");
			continue;
		}

		syslog( LOG_INFO, "UDP Server received a packet...");
		
		if( ((struct sockaddr *) client_addr_buffer)->sa_family == AF_INET )
		{
			client_addr = (char *) malloc(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(((struct sockaddr_in *) client_addr_buffer)->sin_addr), client_addr, INET_ADDRSTRLEN);
			client_port =  ((struct sockaddr_in *) client_addr_buffer)->sin_port;
		}
		else if( ((struct sockaddr *) client_addr_buffer)->sa_family == AF_INET6 )
		{
			client_addr = (char *) malloc(INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(((struct sockaddr_in6 *) client_addr_buffer)->sin6_addr), client_addr, INET6_ADDRSTRLEN);
			client_port =  ((struct sockaddr_in6 *) client_addr_buffer)->sin6_port;
		}
		else
		{
        		syslog( LOG_ERR, "Undefined sa_family %d", ((struct sockaddr *) client_addr_buffer)->sa_family);
			continue;
		}

		/* fork() returns 0 to the child process only */
		if (fork() == 0) {
			
			argument.server_addr = server_addr;
			argument.server_port = server_port;
			argument.client_addr = client_addr;
			argument.client_port = client_port;
			argument.proto.udpArg.socket_addr = (struct sockaddr *) client_addr_buffer;
			argument.proto.udpArg.addr_len = ((struct sockaddr *) client_addr_buffer)->sa_len;
			argument.proto.udpArg.socket = server_socket;
			argument.proto.udpArg.buffer = buffer;
			argument.proto.udpArg.length = numbytes;
			
			(*processor)(&argument); 

			free( client_addr );
			exit(0);
		}
		free( client_addr );
	}
	postprocessor( argument.process_data );

}


char *gs_trim(char *word) {
	char *ptr;

	if (!word)
		return NULL;

	if (!*word)
		return word;

	for (ptr = word + strlen(word) - 1; (ptr >= word) && isspace(*ptr); --ptr);
	
	ptr[1] = '\0';

	for (ptr = word; (ptr <= word + strlen(word) - 1 ) && isspace(*ptr); ++ptr);

	if( ptr != word )
	{
		memcpy( word, ptr, strlen(ptr) +1 );
	}

	return word;
}


char * gs_read_line( FILE * client, char * buf, int trim ){

	char * ret;

	memset( buf, LINE_MAX, 0);

	ret = fgets( buf, LINE_MAX, client);
	if( ret == NULL)
	{
		if( ferror(client) )
			perror( "feof");

		return NULL;
	}

	if( trim == 1)
		return gs_trim(ret);
	else 
		return ret;
}

int gs_word_count(char *line){

	int words;
	int i;	
	int sp;
	line = gs_trim(line);

	words = 0;
	sp = 1;
	
	for( i=0; line[i] != '\0'; i++)
	{
		if(isspace(line[i]))
		{
			sp=1;
		}
		else if(sp) 
		{
			++words;
			sp=0;
		}
	}
	return words; 
}


int parseLine( char *input, char * format, ...)
{
        va_list argList;
	int ret = 1;

        va_start( argList, format);
        ret = vsscanf( input, format, argList);
	va_end(argList);

        return ret;
}


