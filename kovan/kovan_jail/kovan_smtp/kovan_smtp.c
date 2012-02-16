#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdarg.h>

#include "../shared/generic_server.h"
#define MAX_RESP_MSG 32

enum SMTP_STATE { BASE, INITIAL, TRANSACTION, RCPT } state;
enum RESPONSE {  
	UNDEFINED, 
	HELO_250,
	MAIL_250,
	MAIL_SYNTAX_ERR,
	MAIL_NESTED_ERR,
	RCPT_NEED_MAIL,
	RCPT_SYNTAX_ERR,
	RCPT_OK,
	DATA_NEED_RCPT,
	DATA_START,
	DATA_OK,
	QUIT_BYE,
	CMD_NOT_RECOG,
	HELO_FIRST_503
	};

struct sResp {
	enum RESPONSE resp_code;
	char * resp_msg;
};

struct app_data {
	struct sResp * responses[MAX_RESP_MSG];
};


char * configfile;
char * logsocket;
int resp_cnt;

int hflag;
int pflag;
int cflag;

 
int resp_char2enum( char * resp ){

	if( strcmp( resp, "HELO_250" ) == 0 )
	{
		return HELO_250;
	}
	else if( strcmp( resp, "HELO_FIRST_503" ) == 0 )
	{
		return HELO_FIRST_503;
	}
	else if( strcmp( resp, "MAIL_250" ) == 0 )
	{
		return MAIL_250;
	}
	else if( strcmp( resp, "MAIL_SYNTAX_ERR" ) == 0 )
	{
		return MAIL_SYNTAX_ERR;
	}
	else if( strcmp( resp, "MAIL_NESTED_ERR" ) == 0 )
	{
		return MAIL_NESTED_ERR;
	}
	else if( strcmp( resp, "QUIT_BYE" ) == 0 )
	{
		return QUIT_BYE;
	}
	else if( strcmp( resp, "RCPT_NEED_MAIL" ) == 0 )
	{
		return RCPT_NEED_MAIL;
	}
	else if( strcmp( resp, "RCPT_SYNTAX_ERR" ) == 0 )
	{
		return RCPT_SYNTAX_ERR;
	}
	else if( strcmp( resp, "RCPT_OK" ) == 0 )
	{
		return RCPT_OK;
	}
	else if( strcmp( resp, "DATA_NEED_RCPT" ) == 0 )
	{
		return DATA_NEED_RCPT;
	}
	else if( strcmp( resp, "DATA_START" ) == 0 )
	{
		return DATA_START;
	}
	else if( strcmp( resp, "DATA_OK" ) == 0 )
	{
		return DATA_OK;
	}
	else if( strcmp( resp, "CMD_NOT_RECOG" ) == 0 )
	{
		return CMD_NOT_RECOG;
	}
	else
		return UNDEFINED;
}

void send_response( FILE * client, struct app_data * shared_data, enum RESPONSE respCode, ...)
{
	char buf[LINE_MAX];
	va_list argList;
	int ret;
	int i;

	memset( buf, 0, LINE_MAX);
	for( i=0; i < resp_cnt; i++)
	{
		if( shared_data->responses[i]->resp_code == respCode )
		{
			break;
		}
	}

	if( i == resp_cnt )
	{
		printf("send_response: code not found\n");
		return;
	}	

	va_start( argList, respCode);
	sprintf(buf, shared_data->responses[i]->resp_msg, argList );
	ret = fwrite(buf , 1 , strlen(buf) , client);
	ret = fwrite("\r\n" , 1 , strlen("\r\n") , client);
	va_end(argList);
}

void postprocess( char * data ){

	struct app_data * pdata = (struct app_data *)data;

	int i;

	for( i=0; i < resp_cnt ; i++)
	{
		free(pdata->responses[i]->resp_msg);
		free(pdata->responses[i]);
	}

	free(pdata);
}

char * preprocessor( int * retcode ){

	char * ret;
	char *buf;
	FILE * file;
	char cmd[32];
	char *msg;
	int code;
	char * name;
	struct app_data * shared_data;

	name = configfile;
	shared_data = (struct app_data *) malloc( sizeof( struct app_data));

	
	/* don't forget to free */
	buf = (char *) malloc(LINE_MAX*sizeof(char));

	file = fopen( name, "r" );
	if( file == NULL )
	{
		syslog( LOG_ERR, "Unable to open config file %s: %m", name);
		*retcode = -1;
		return NULL;
	}

	syslog( LOG_INFO, "Reading config file");

	while( !feof(file))
	{
		memset( buf, LINE_MAX, 0);

		ret = fgets( buf, LINE_MAX, file);

		buf = gs_trim(buf); 

		if( strlen(buf) > 0 ){

			if( parseLine( buf, "%s%*[ ]",cmd) == 0 )
			{
				syslog( LOG_ERR,"Failed to parse cmd. Line: %s", buf);
				continue;
			}

			msg = strchr( buf,  '"');
			msg = msg + 1;

			msg[strlen(msg)-1] = '\0';
			shared_data->responses[resp_cnt] = ( struct sResp *) malloc( sizeof(struct sResp));
			code = resp_char2enum( cmd );

			if( code == UNDEFINED )
			{
				syslog( LOG_ERR,"Undefined command %s", cmd);
			}
			else
			{
				shared_data->responses[resp_cnt]->resp_code = code;
				shared_data->responses[resp_cnt]->resp_msg = (char *) malloc( (strlen(msg)+1)*sizeof(char) );
				memcpy( shared_data->responses[resp_cnt]->resp_msg, msg, strlen(msg)+1);
				resp_cnt++;	
			}
		}

	}
	syslog( LOG_INFO, "Config file is read...");

	free(buf);

	fclose(file);
	*retcode = 0;
	return (char *) shared_data; 
}

int smtp_server( ProcArg * argument  ){

	char *ptr;	
	char buf[LINE_MAX];
	char *ret;
	int loop;
	int size;
	char cmd[5];
	char from[16];
	char arg[256];
	char *rcpts[32];
	int rcpt_cnt;
	int i;
	int data_size;
	char * data;
	struct app_data * shared_data;
        char rcptlist[BUFFER_1K];
	int total_size;

	/* do not forget to free dynamic memory */
	char *domain;

	FILE * client = argument->proto.tcpArg.client;
	shared_data = (struct app_data *) argument->process_data ;

	syslog( LOG_INFO, "Starting SMTP Server...");

	state = BASE;

	rcpt_cnt = 0;

	domain = NULL;

	for( loop = 0; (ret = gs_read_line(client, buf, 1)) != NULL; loop++)
	{
		/* give error in case fist message is not helo */

		if( parseLine( buf, "%s ", cmd) != 0 )
		{
			/* minimum set of commands are implemented according to RFC 821 */
			if( strlen(cmd) != 4 )
			{
				send_response( client, shared_data, CMD_NOT_RECOG);
				continue;
			}

			if( strncasecmp( cmd, "HELO", 4) == 0 )
			{
				memset(cmd, 0, 5);
				memset(arg, 0, 256);

				if(parseLine( buf, "%*4c%*[ ]%s", arg) != 0 )
				{
					state = INITIAL;
					domain = (char *) malloc( (strlen(arg)+1) * sizeof(char));
					memcpy( domain, arg, strlen(arg) +1 );
			
					/* ! check succesfullness .. */	
					send_response( client, shared_data, HELO_250 );
				}
			}
			else if( strncasecmp( cmd, "MAIL", 4) == 0)
			{
				if( loop == 0 ){
					send_response( client, shared_data, HELO_FIRST_503); 
					continue;
				}
				if( state == TRANSACTION){
					send_response( client, shared_data, MAIL_NESTED_ERR); 
					continue;
				}

				memset(cmd, 0, 5);
				memset(arg, 0, 256);

				if(parseLine( buf, "%*4c%*[ ]%s", arg) != 0 )
				{
					if( strncasecmp( arg, "FROM:", 5) == 0 )
					{
						if(strlen(arg) == 5)
						{
							send_response( client, shared_data, MAIL_SYNTAX_ERR);
						}
						else
						{
							if( strchr( arg, '<') )
							{
								ptr = strchr( arg, '<') + 1;
								size = strlen( ptr );
								ptr[size -1] = '\0';
								memcpy( from, ptr, strlen(ptr) +1);
							} 
							else
							{
								ptr = strchr( arg, ':') + 1;
								memcpy( from, ptr, strlen(ptr) +1);
							}
							state = TRANSACTION;
							send_response( client, shared_data, MAIL_250);
						}
					}
					else
					{
						send_response( client, shared_data, MAIL_SYNTAX_ERR);
					}

					
				}
			}
			else if( strncasecmp( cmd, "RCPT", 4) == 0)
			{
				if( state != TRANSACTION && state != RCPT ){
					send_response( client, shared_data, RCPT_NEED_MAIL); 
					continue;
				}

				memset(cmd, 0, 5);
				memset(arg, 0, 256);

				if(parseLine( buf, "%*4c%*[ ]%s", arg) != 0 )
				{
					if( strncasecmp( arg, "TO:", 3) == 0 )
					{
						if(strlen(arg) == 3)
						{
							send_response( client, shared_data, RCPT_SYNTAX_ERR);
						}
						else
						{
							if( strchr( arg, '<') )
							{
								ptr = strchr( arg, '<') + 1;
								size = strlen( ptr );
								ptr[size -1] = '\0';
							} 
							else
							{
								ptr = strchr( arg, ':') + 1;
							}
							rcpts[rcpt_cnt] = (char *) malloc( (strlen(ptr)+1)*sizeof(char));
							memcpy( rcpts[rcpt_cnt], ptr, strlen(ptr) +1);
							state = RCPT;
							rcpt_cnt++;
							send_response( client, shared_data, RCPT_OK, shared_data );
						}
					}
					else
					{
						send_response( client, shared_data, RCPT_SYNTAX_ERR);
					}
				}
			}
			else if( strncasecmp( cmd, "DATA", 4) == 0)
			{
				if( state != RCPT )
				{
					send_response( client, shared_data, DATA_NEED_RCPT);
					continue;
				}
				else if( state == RCPT )
				{
					send_response( client, shared_data, DATA_START);
					data_size = 0;
					data = NULL;

					while( gs_read_line(client, buf,0) != NULL )
					{
						size = strlen(buf);  
						data = (char *) realloc( data, data_size+size );
						
						if( data == NULL )
							perror("ERROR\n");

						memcpy( data + data_size, buf, size );
						data_size += size;

						if( strstr( data, "\r\n.\r\n") != NULL )
						{
							break;
						}
					}
					send_response( client, shared_data, DATA_OK);
					state = TRANSACTION;
				}
			}
			else if( strncasecmp( cmd, "RSET", 4) == 0)
			{
				state = BASE;
				free(domain);
			}
			else if( strncasecmp( cmd, "NOOP", 4) == 0)
			{

			}
			else if( strncasecmp( cmd, "QUIT", 4) == 0)
			{
				send_response( client, shared_data, QUIT_BYE);
				break;
			}else
				send_response( client, shared_data, CMD_NOT_RECOG);
		}
		else
		{
			syslog( LOG_INFO, "Parse error for cmd: %s", cmd);
		}


	}

	memset( rcptlist, 0, BUFFER_1K);
	total_size = 0;
	for( i=0; i<rcpt_cnt; i++)
	{
		if( (total_size + strlen(rcpts[i]) + 2) >= BUFFER_1K )
		{
			syslog( LOG_INFO, "Total size of receipts exceed the bound %d", BUFFER_1K);
			break;
		}
		strcat( rcptlist, rcpts[i]);
		strcat( rcptlist, " ");
		total_size += strlen(rcpts[i]) + 2;
	}
	
	// syslog( LOG_WARNING, "%s [connect from %s] [mail from %s] [rcpt to %s]", date_str, argument->client_addr, from, rcptlist );
	time_t seconds;
	seconds = time (NULL);

        syslog( LOG_WARNING, "%s %d %s %d tcp \"%s %s %s\" %ld", argument->client_addr, argument->client_port,
                                                             argument->server_addr, argument->server_port,
                                                             from, rcptlist, data, seconds);


	if( domain != NULL )
		free(domain);
	
	for( i=0; i<rcpt_cnt; i++)
		free( rcpts[i] );
	
	syslog( LOG_INFO, "SMTP Server finished...");

	return 1;

}

int main(int argc, char *argv[])
{
	char *host;
	char *port;
	char *pidfile;
	
	int ch = 0;
	hflag = 0;
	pflag = 0;
	cflag = 0;
	pidfile = NULL;

        while ((ch = getopt(argc, argv, "h:p:c:d:")) != -1)
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
		case 'c':
			cflag = 1;
                        configfile = (char *) malloc(strlen(optarg) + 1);
                        strcpy(configfile, optarg);
                        break;
		case 'd':
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

	if( pflag == 0 || hflag == 0 || cflag==0)
	{
		printf( "usage: kovan_smtp [-d pidfile] -h host -p port -c config\n");
		return 1;
	}

	gs_start( TCP, pidfile, host, port, smtp_server, preprocessor, postprocess);
	
	free(host);
	free(port);
	free(configfile);
	free(pidfile);
	free(logsocket);

	return 0;
}

