#include <stdio.h>
#include <sys/socket.h>

#define MAXSOCK 5
#define BUFFER_1K 1024
#define BUFFER_256 256
#define BUFFER_64 64 
#define BUFFER_32 32


/* 
 * This type will be used when creating applications on top of
 * generic server. 
 */
typedef enum{ TCP, UDP } ServerType;

/*
 * This structure is created and filled before upper layer tcp processor
 * is called. Filled object is used in upper layer. Upper layer application
 * such as http, smtÄp, ftp ..etc is responsible of the object.
 */
typedef struct {
        FILE * client;
} TCPArg;


/*
 * This structure is created and filled before upper layer udp processor
 * is called. Filled object is used in upper layer. Upper layer application
 * such as dns is responsible of the object. 
 */
typedef struct {
        struct sockaddr *socket_addr;
	socklen_t addr_len;
        int socket;   
        int length;   
        char * buffer;
} UDPArg;

/*
 * An upper layer application must either a tcp or udp application therefore
 * upper layer processer function will take only one kind of argument. 
 * Union is used here for less memory usage.
 */
typedef struct {
	char * server_addr;
	char * client_addr;
	int server_port;
	int client_port;
	union{ 
        	TCPArg tcpArg;
        	UDPArg udpArg;
	} proto;
	char * process_data;
} ProcArg;



void gs_close( int sig );
void gs_start( ServerType eServerType, char * pidfile, const char * ip_addr, const char * port, int (*processor)(ProcArg *), char * (*preprocessor)( int *), void (*postprocessor)(char *) );
void gs_tcp( int socket, int (*processor)(ProcArg *), char * (*preprocessor)(int *), void (*postprocessor)(char *) );
void gs_udp( int socket, int (*processor)(ProcArg *), char * (*preprocessor)(int *), void (*postprocessor)(char *) );
char * gs_trim(char *word);
int gs_word_count(char *line);
char * gs_read_line( FILE * client, char * buf , int trim);
int parseLine( char *input, char * format, ...);

