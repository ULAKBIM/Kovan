#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>


#include "../shared/generic_server.h"
int hflag;
int pflag;
char * logsocket;

void log_activity( ProcArg * argument  ){
        printf("%s %d -> %s %d\n", argument->server_addr, argument->server_port, argument->client_addr, argument->client_port);
}

void postprocess( char * data ){
	/* do nothing */
}

char * preprocessor( int * retcode ){
	/* do nothing */
	*retcode = 0;
	return NULL;
}


int ftp_server( ProcArg * argument  ){

	char buf[LINE_MAX];
	char cmd[BUFFER_64];
	char arg[BUFFER_64];
	char usr[BUFFER_64];
	char *ret;
	FILE * client = argument->proto.tcpArg.client;
	char * response;
	int quit;
	time_t rawtime;
	struct tm * timeinfo;


	quit = 0;
	while( (ret = gs_read_line(client, buf, 1)) != NULL && quit != 1)
	{	
		memset( cmd, 0, BUFFER_64 );
		memset( arg, 0, BUFFER_64 );
		parseLine( buf, "%s%*[ \t]%s", cmd, arg);

		if( strcasecmp(cmd, "QUIT") == 0)
		{
			response = "221 Goodbye.\r\n";
			quit = 1;
		}
		else if( strcasecmp(cmd, "SYST") == 0)
		{
			response = "215 UNIX Type: L8\r\n";
		}
		else if( strcasecmp(cmd, "HELP") == 0)
		{
response = "214-The following commands are recognized (* =>'s unimplemented).\r\n\
   USER    PORT    STOR    MSAM*   RNTO    NLST    MKD     CDUP\r\n\
   PASS    PASV    APPE    MRSQ*   ABOR    SITE    XMKD    XCUP\r\n\
   ACCT*   TYPE    MLFL*   MRCP*   DELE    SYST    RMD     STOU\r\n\
   SMNT*   STRU    MAIL*   ALLO    CWD     STAT    XRMD    SIZE\r\n\
   REIN*   MODE    MSND*   REST    XCWD    HELP    PWD     MDTM\r\n\
   QUIT    RETR    MSOM*   RNFR    LIST    NOOP    XPWD\r\n\
214 Direct comments to ftp@fomain.\r\n";

		}
		else if( strcasecmp(cmd, "USER") == 0)
		{
			if( strcasecmp(arg, "ANONYMOUS") == 0)
			{
				response = "331 Guest login ok, send your complete e-mail address as a password.\r\n";
				memcpy( usr, "ANONYMOUS", strlen("ANONYMOUS")+1);
			}
			else
			{
				memcpy( usr, arg, strlen(arg)+1);
				sprintf( buf, "331 Password required for %s.\r\n", arg);
				response = buf;
			}
		}
		else if( strcasecmp(cmd, "PASS") == 0)
		{
			if( strcasecmp(usr, "ANONYMOUS") == 0)
			{

				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
response = "230-Hello User at =???,\r\n\
230-we have 911 users (max 1800) logged in in your class at the moment.\r\n\
230-Local time is: %s\r\n\
230-All transfers are logged. If you don't like this, disconnect now.\r\n\
230-\r\n\
230-tar-on-the-fly and gzip-on-the-fly are implemented; to get a whole\r\n\
230-directory \"foo\", \"get foo.tar\" or \"get foo.tar.gz\" may be used.\r\n\
230-Please use gzip-on-the-fly only if you need it; most files already\r\n\
230-are compressed, and I will kill your processes if you waste my\r\n\
230-ressources.\r\n\
230-\r\n\
230-The command \"site exec locate pattern\" will create a list of all\r\n\
230-path names containing \"pattern\".\r\n\
230-\r\n\
230 Guest login ok, access restrictions apply.\r\n";
				sprintf( buf, response, gs_trim(asctime(timeinfo)));
				response = buf;
			}
			else
			{
				response = "530 Login incorrect.\r\n";
			}
		}
		else if( strcasecmp(cmd, "MKD") == 0)
		{
			sprintf( buf, "257 %s new directory created.\r\n", arg);
			response = buf;
		}
		else if( strcasecmp(cmd, "CWD") == 0)
		{
			response = "250 CWD command successful.\r\n";
		}
		else if( strcasecmp(cmd, "NOOP") == 0)
		{
			response = "200 NOOP command successful.\r\n";
		}
		else if( strcasecmp(cmd, "PASV") == 0)
		{
			response = "227 Entering Passive Mode (134,76,11,100,165,53.\r\n";
		}
		else if( strcasecmp(cmd, "ACCT") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "SMNT") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "REIN") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MLFL") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MAIL") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MSND") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MSON") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MSAM") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MRSQ") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MRCP") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else if( strcasecmp(cmd, "MLFL") == 0)
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}
		else
		{
			sprintf( buf, "502 %s command not implemented.\r\n", cmd);
			response = buf;
		}

		fwrite (response , 1 , strlen(response)+1 , client);
		memset( cmd, 0, 64);
		memset( arg, 0, 64);
		if( quit == 1 )
		{
			break;
		}

		// syslog( LOG_WARNING, "%s [from %s[%s]] %s", date_str, argument->client_addr, argument->client_addr, buf );
		time_t seconds;
		seconds = time (NULL);

	        syslog( LOG_WARNING, "%s %d %s %d tcp \"%s\" %ld", argument->client_addr, argument->client_port,
                                                             argument->server_addr, argument->server_port,
                                                             buf, seconds);


	}

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
	pidfile = NULL;

        while ((ch = getopt(argc, argv, "h:p:c:")) != -1)
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
        if( pflag == 0 || hflag == 0 )
        {
                printf( "usage: kovan_ftp [-d pidfile] -h host -p port \n");
                return 1;
        }

        gs_start( TCP, pidfile, host, port, ftp_server, preprocessor, postprocess);

        free(host);
        free(port);
	free(pidfile);
	free(logsocket);

	return 0;
}
