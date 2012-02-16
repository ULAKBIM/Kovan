#include "../shared/generic_server.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <syslog.h>
#include <stdarg.h> 
#include <unistd.h>

#define MAX_FILE_NAME 512

char * webroot;
char *logsocket;
char * default_file = "/index.html";

int hflag;
int pflag;
int wflag;
int dflag;

void postprocess( char * data ){
       /* do nothing */
}

char * preprocessor( int * retcode ){
        /* do nothing */
	*retcode = 0;
        return NULL;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  writeResponseHeader
 *  Description:  Method takes response status, response filetype and client file
 *  handle as argument and send response message to client.
 * =====================================================================================
 */
void writeResponseHeader( int status, char * fileType, FILE * client )
{

	int bytes;
	char * line;

	switch( status ){

		case 200:
			line = "HTTP/1.1 200 OK\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Date: Mon, 19 Jul 2010 10:25:30 GMT\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Server: Apache\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Last-Modified: Mon, 05 Nov 2007 13:36:09 GMT\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Connection: close\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Content-Type: ";
			bytes = fwrite(line , 1 , strlen(line) , client);
			bytes = fwrite(fileType , 1 , strlen(fileType) , client);
			bytes = fwrite("\r\n" , 1 , strlen("\r\n") , client);
			break;
		case 404:
			line = "HTTP/1.1 404 Not Found\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Date: Mon, 19 Jul 2010 10:25:30 GMT\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Server: Apache\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "X-Powered-By: PHP/5.2.6";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Connection: close\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);
			line = "Content-Type: text/html\r\n";
			bytes = fwrite(line , 1 , strlen(line) , client);

		default:
			break;

	}
	bytes = fwrite("\r\n" , 1 , strlen("\r\n") , client);

}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  responseFile
 *  Description:  Method takes requested file name and client file descriptor as argument
 *  and sends requested file to client. 
 * =====================================================================================
 */
int responseFile( char * req_file, FILE * client, int *objSize )
{
	int ret;
	int bytes;
	struct stat sbuf;
	int read;
	char * type;
	char buf[BUFFER_1K];
	char fileName[MAX_FILE_NAME];
	FILE * file;

	/*
	 * Initialize filename to all zeros. Concat webroot and 
	 * requested filename to get fullpath of the file. 
	 */
	memset( fileName, 0, MAX_FILE_NAME);
	strncat( fileName, webroot, strlen(webroot));
	strncat( fileName, req_file, strlen(req_file));

	ret = stat( fileName, &sbuf );
	if ( ret != 0 )
	{
		writeResponseHeader( 404, "", client );
		ret = 404;
		*objSize = 0;
	}
	else
	{
		*objSize = sbuf.st_size;

		file = fopen( fileName, "r" );
		type = strchr( fileName,  '.');	

		if( strncmp( type, ".html", strlen(".html")) == 0)
		{
			type = "text/html";
		}
		else if( strncmp( type, ".jpeg", strlen(".html")) == 0)
		{
			type = "image/jpeg";
		}

		writeResponseHeader( 200, type, client );

		memset( buf, BUFFER_1K, 0);

		while( !feof(file) )
		{
			read = fread( buf, 1, BUFFER_1K, file);
			bytes = fwrite(buf , 1 , read , client);
			memset( buf, BUFFER_1K, 0);
		}

		fflush( client );
		fclose( file );
		ret = 200;
	}

	return ret;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  http_server
 *  Description:  This core method processes incomming HTTP requests and calls responseFile
 *  method to send requests to responses.
 * =====================================================================================
 */
int http_server( ProcArg * argument  ){
	
	char *ret;
	int rcode;
	int loop;
	char buf[LINE_MAX];
	char method[BUFFER_256];
	char req_uri[BUFFER_256];
	char keyword[BUFFER_256];
	char value[BUFFER_256];
	char http_ver[BUFFER_32];
	int keep_connection = 0;
	int objsize;

	FILE * client = argument->proto.tcpArg.client;

	/*
	printf("Starting HTTP Server...\n");
	*/

	for( loop = 1; (ret = gs_read_line(client, buf, 1)) != NULL; loop++)
	{
		/*  check if this line is the end of the http request */	
		if( strncmp( buf, "\r\n", strlen(buf)) == 0)
			break;

		/*  at first line; read method, request uri and http_ver */
		if( loop == 1 )
		{
			if( parseLine( buf, "%s %s %s", method, req_uri, http_ver) != 0 )
			{
				/*
				printf("method: '%s' uri: '%s' http: '%s'\n", method, req_uri, http_ver);
				*/
			}

		}
		else
		{
			if( parseLine( buf, "%s %s", keyword, value) != 0 )
			{
				/*
				printf("Keyword: '%s' Value: '%s'\n", keyword, value);
				*/

				if( strncmp( keyword, "Connection:", strlen("Connection:")) == 0)
				{
					if( strncmp( value, "keep-alive", strlen("keep-alive")) == 0)
					{
						keep_connection = 1;
					}
				}
			}
		}
	}

	if( strncmp( method, "GET", strlen("GET")) == 0)
	{
		if( strncmp( req_uri, "/", strlen(req_uri)) == 0)
			rcode = responseFile( default_file, client, &objsize);
		else
			rcode = responseFile( req_uri, client, &objsize);
	}

	/* 127.0.0.1 - frank [10/Oct/2000:13:55:36 -0700] "GET /apache_pb.gif HTTP/1.0" 200 2326 */

	time_t seconds;
	seconds = time (NULL);

        syslog( LOG_WARNING, "%s %d %s %d tcp \"%s %s %s %d %d\" %ld", argument->client_addr, argument->client_port,
                                                             argument->server_addr, argument->server_port,
                                                             method, req_uri, http_ver,
                                                             rcode, objsize, seconds );

/*	
	syslog( LOG_WARNING, "%s %s %s [%s] \"%s %s %s\" %d %d", 
						argument->server_addr, 
						"-", 
						"root", 
						date_str, 
						method, req_uri, http_ver,
						rcode,
						objsize );
						
*/
	return 1;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: main
 *  Description: Function initalizes base variable to be used by core functions. Host 
 *  IP and PORT must be given as arguments to program. If there are missing arguments
 *  program exits with warning message and returns 1. Otherwise, core method is called.
 * =====================================================================================
 */
int
main(int argc, char *argv[])
{

	char *host;
	char *port;
	char *pidfile;
	
	int ch = 0;
	hflag = 0;
	pflag = 0;
	dflag = 0;
	pidfile = NULL;
	logsocket = NULL;

        while ((ch = getopt(argc, argv, "h:p:w:d:l:")) != -1)
        {
                switch(ch)
                {
                case 'h':
                        hflag = 1;
                        host = (char *) malloc(strlen(optarg) + 1);
                        strcpy(host, optarg);
                        break;
                case 'p':
                        pflag = 1;
                        port = (char *) malloc(strlen(optarg) + 1);
                        strcpy(port, optarg);
                        break;
                case 'w':
                        wflag = 1;
                        webroot = (char *) malloc(strlen(optarg) + 1);
                        strcpy(webroot, optarg);
                        break;
		case 'd':
			dflag = 1;
                        pidfile = (char *) malloc(strlen(optarg) + 1);
                        strcpy(pidfile, optarg);
			break;
		case 'l':
                        logsocket = (char *) malloc(strlen(optarg) + 1);
                        strcpy(logsocket, optarg);
			break;
		default:
			break;
		}

	}

	if( pflag == 0 || hflag == 0 || wflag == 0 )
	{
		printf( "usage: kovan_http [-d pidfile] -h host -p port -w webroot\n");
		return 1;
	}

	gs_start( TCP, pidfile, host, port, http_server, preprocessor, postprocess);
	
	free(host);
	free(port);
	free(pidfile);
	free(webroot);
	free(logsocket);

	return 0;
} 				/* ----------  end of function main  ---------- */
