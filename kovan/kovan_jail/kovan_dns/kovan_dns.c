#include "../shared/generic_server.h"
#include "kovan_dns.h"

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <netinet/in.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>


#define LABEL_LEN 64
#define DOMAIN_LEN 256 

char * configfile;
int hflag;
int pflag;
int cflag;
int rflag;
int recursion = 0;  /*  recursion unavailable  */
void postprocess( char * data );

void print_zones( struct app_data * pdata)
{
	int i, j ;

	for( i=0; i< pdata->zone_cnt; i++)
	{
		printf("Zone: '%s' TTL: '%s'\n", pdata->zone_list[i]->name, pdata->zone_list[i]->ttl);
		printf("*******Records*******\n");

		for( j=0; j < pdata->zone_list[i]->record_cnt; j++)
		{
			printf( "'%s'\n", pdata->zone_list[i]->rrlist[j]->dname );
		}
	}
}

char * preprocessor( int * retcode ){

	char domain[DOMAIN_LEN];
	char zonefile_prefix[PATH_MAX];
	char zonefile_name[PATH_MAX];
	char zonefile_postfix[PATH_MAX];
	int length;
	char * fname;
	char * ptr;
	struct app_data * shared_data;

	fname = configfile;
	shared_data = (struct app_data *) malloc( sizeof( struct app_data));
	memset(shared_data, 0, sizeof( struct app_data));

	FILE * file;
	char line[LINE_MAX];

	syslog( LOG_INFO,"Parsing config file %s",fname);
	
	file = fopen( fname, "r");
		
	if( file == NULL ){
		syslog( LOG_ERR, "file '%s' can not be opened: %m",fname);
		*retcode = -1;
		return NULL;
	}
	
	syslog( LOG_INFO,"Config file found...");
	shared_data->zone_cnt = 0;
	

	memset( zonefile_prefix, 0, PATH_MAX);
	memcpy( zonefile_prefix, fname, strlen(fname)+1);
	ptr = strrchr( zonefile_prefix, '/');
	
	if( ptr == NULL )
	{
		syslog( LOG_ERR, "Give full path of configuration file %s",fname);
		postprocess( (char *) shared_data );
		fclose(file);
		*retcode = -1;
		return NULL;
	}
	else if( *(ptr+1) == '\0')
	{
		syslog( LOG_ERR, "Given config file name %s has '/' char at end",fname);
		postprocess( (char *) shared_data );
		fclose(file);
		*retcode = -1;
		return NULL;
	}
	else
	{
		*(ptr+1) = '\0';
	}


	while( gs_read_line(file, line, 1) )
	{
		memset( domain, 0, DOMAIN_LEN);
		memset( zonefile_postfix, 0, PATH_MAX);
		
		
		if( parseLine( line, "%s%*[ ]%s", domain, zonefile_postfix) == 0 )
		{
			syslog( LOG_ERR, "Failed to parse zone and related zonefilename. Config line: %s", line);
			postprocess( (char *) shared_data );
			fclose(file);
			*retcode = -1;
			return NULL;
		}
	
		/* check invalid syntax */
		memcpy( domain, domain+1, strlen(domain));
		length = strlen(domain);
		domain[length-1] = '\0';
	
		memcpy( zonefile_postfix, zonefile_postfix+1, strlen(zonefile_postfix));
		length = strlen(zonefile_postfix);
		zonefile_postfix[length-1] = '\0';

		memcpy(zonefile_name, zonefile_prefix, strlen(zonefile_prefix));
		memcpy(zonefile_name+strlen(zonefile_prefix), zonefile_postfix, strlen(zonefile_postfix)+1);
		printf("%s\n",zonefile_name);

		shared_data->zone_list = ( struct zone **) realloc(shared_data->zone_list, sizeof(struct zone *)*(shared_data->zone_cnt+1));
		shared_data->zone_list[shared_data->zone_cnt] = ( struct zone *) malloc( sizeof(struct zone));
		memset( shared_data->zone_list[shared_data->zone_cnt], 0, sizeof(struct zone));

		shared_data->zone_list[shared_data->zone_cnt]->name = (char *) malloc( (strlen(domain)+1) * sizeof(char));
		memcpy( shared_data->zone_list[shared_data->zone_cnt]->name, domain, strlen(domain)+1);
		shared_data->zone_cnt++;

		if( read_zone( zonefile_name, shared_data->zone_list[shared_data->zone_cnt-1]) != 0 )
		{
			syslog( LOG_ERR, "zonefile %s read error", zonefile_name);
			postprocess( (char *) shared_data );
			fclose(file);
			*retcode = -1;
			return NULL;
		} 
	}

	syslog( LOG_INFO,"Config file %s is parsed", fname);
	fclose( file );
	*retcode = 0;
	return (char *) shared_data;
}

int read_zone( char * fname, struct zone * zone)
{
	FILE * file;
	int size;
	char * ptr;
	char line[LINE_MAX];
	char token[DOMAIN_LEN];
	char token2[DOMAIN_LEN];
	char *origin;
	int record_cnt;
	struct rrecord * recptr;

	file = fopen( fname, "r");
	if( file == NULL ){
		syslog( LOG_ERR,"file '%s' can not be opened: %m",fname);
		return -1;
	}
	
	record_cnt = 0;	
	/* 
	 * Read and parse each line. In normal case each loop
	 * reads one line but for multiline records, all lines
	 * belong to a single record are read
	 */ 
	while( gs_read_line(file, line, 0) )
	{
		/* ignore characters after ';' */
		ptr = strchr( line, ';');
		if( ptr != NULL )
			*ptr = '\0';

		/* clean trailing and preciding white spaces */
		ptr = gs_trim(line);
		memcpy( line, ptr, strlen(ptr) +1);

		/* if line is empty ignore it and continue next line */
		if( strcmp( line, "") == 0 )
			continue;

		/* try to parse first token, print alert if error occurs */
		if( parseLine( line, "%s", token) == 0)
		{
			syslog( LOG_ERR,"Failed to parse first token. Line: %s", line);
			continue;
		}

		/* 
		 * In zonefile we have two possible special first tokens 
		 * $ORIGIN keyword or $TTL keyword
		 */
		if( strncmp( token, "$ORIGIN", strlen("$ORIGIN")) == 0 )
		{
			/* parse ORIGIN value and copy into origin */
			if( parseLine( line, "%*s%*[ ]%s", token) == 0 )
			{
				syslog( LOG_ERR,"Failed to parse $ORIGIN. Line: %s", line);
				continue;
			}

			origin = (char *) malloc( sizeof(char) * (strlen(token) + 1));
			memcpy( origin, token, strlen(token) +1 );
			continue;
		}
		else if( strncmp( token, "$TTL", strlen("$TTL")) == 0 )
                {
			/* parse TTL value and copy into zone's ttl field */
                        if( parseLine( line, "%*s%*[ ]%s", token) == 0)
			{
				syslog( LOG_ERR,"Failed to parse $TTL. Line: %s", line);
				continue;
			}

			zone->ttl = (char *) malloc( sizeof(char) * (strlen(token) + 1));
			memcpy( zone->ttl, token, strlen(token) + 1);
			continue;
                }

		/* Allocate a free room in rrlist and parse resource record to fill it*/
		zone->rrlist =  (struct rrecord **) realloc( zone->rrlist, sizeof(struct rrecord *) *(record_cnt + 1));
		zone->rrlist[record_cnt] = (struct rrecord *) malloc( sizeof(struct rrecord));
		recptr = zone->rrlist[record_cnt];

		/*
		 * Get first token which is domain name in normal conditions. 
		 * Domain name has three possibilities name, name. and @ . 'name'
		 * is subdomain and in order to get full domain name concatenation 
		 * with origin is required, name.origin.com. 'name.' is root level
		 * domain and no postprocessing is requried. '@' is an alias to 
		 * ORIGIN. 
  		 */
		if( parseLine( line, "%s", token) == 0)
		{
			syslog( LOG_ERR,"Failed to parse $TTL. Line: %s", line);
			continue;
		}
		
		if( strcmp( token, "@") == 0 )
		{
			/* copy origin value to recource records name field. */
			size = strlen(origin) + 1;

			recptr->dname = (char *) malloc( sizeof(char) * size);
			memcpy( recptr->dname, origin, size);
		}
		else 
		{
			ptr = strrchr( token, '.');
			if( ptr == NULL || *(ptr+1) != '\0' )
			{
				/* 
				 * Domain name is a sub domain thus it has to
				 * be concatenated to origin as : name.origin
				 * The '.' character at the end of the origin
				 * has to be deleted. 
				 * dname = 'token' + '.' + 'origin' - '.'
			 	*/
		
				size = strlen(token) + 1 + strlen(origin) + 1;
				recptr->dname = (char *) malloc( sizeof(char) * size);
				memset( recptr->dname, 0, size );
				memcpy( recptr->dname, token, strlen(token));
				memcpy( recptr->dname + strlen(token) , ".", 1);
				memcpy( recptr->dname + strlen(token)+1, origin, strlen(origin) + 1);
			}
			else if( ptr != NULL && *(ptr+1) == '\0')
			{
				/*
				 * Domain name is a root level name thus, only
				 */
				recptr->dname = (char *) malloc( sizeof(char) * strlen(token) + 1 );
                                memcpy( recptr->dname, token, strlen(token) + 1);
			}
			
		}
	
		/* parse second token, it can be either CLASS or RR type */

		if( parseLine( line, "%*s%*[ \t]%s", token) == 0)
		{
			syslog( LOG_ERR,"Failed to parse second token(should be class or type). Line: %s", line);
			continue;
		}

		if( (strcmp( token, "IN") == 0) || (strcmp( token, "CHAOS") == 0) )
		{
			/* class = token */
			recptr->class = (char *) malloc( sizeof(char) * (strlen(token) + 1) );
			memcpy( recptr->class, token, strlen(token) + 1);

			/* rr type = token */
			if( parseLine( line, "%*s%*[ \t]%*s%*[ ]%s", token) == 0)
			{
				syslog( LOG_ERR,"Failed to parse third token(record type). Line: %s", line);
				continue;
			}

			recptr->type = type_str2int( token );
		}
		else
		{
			/* rr type = token */
			if( parseLine( line, "%*s%*[ \t]%s", token) == 0)
			{
				syslog( LOG_ERR,"Failed to parse second token(record type). Line: %s", line);
				continue;
			}

			recptr->type = type_str2int( token );
		}

		if( strcmp( token, "SOA") == 0)
		{
			/* ulakbim.gov.tr. IN  SOA ns1.ulakbim.gov.tr. hostmaster.ulakbim.gov.tr. 2009082702 28800 7200 604800 600 */
			recptr->add_data = (struct soa_data *) malloc(sizeof(struct soa_data));

			if( parseLine( line, "%*s%*[ \t]%*s%*[ \t]%*s%*[ \t]%s%*[ \t]%s%*[ \t]%d%*[ \t]%d%*[ \t]%d%*[ \t]%d%*[ \t]%d",
				token, token2, &recptr->add_data->serial, &recptr->add_data->refresh, &recptr->add_data->retry, 
				&recptr->add_data->expire, &recptr->add_data->minimum) == 0)
			{
				syslog( LOG_ERR,"Failed to parse SOA line. Line: %s", line);
				continue;
			}

			recptr->add_data->mname = (char *) malloc( sizeof(char) * (strlen(token) + 1) );
			memcpy( recptr->add_data->mname, token, strlen(token) + 1);
			
			recptr->add_data->rname = (char *) malloc( sizeof(char) * (strlen(token2) + 1) );
			memcpy( recptr->add_data->rname, token2, strlen(token2) + 1);
		}
		else if( strcmp( token, "MX") == 0)
		{
			if( parseLine( line, "%*s%*[ \t]%*s%*[ \t]%d%*[ \t]%s", &recptr->priority,token) == 0)
			{
				syslog( LOG_ERR,"Failed to parse priority and value. Line: %s", line);
				continue;
			}

			recptr->data = (char *) malloc( sizeof(char) * (strlen(token) + 1) );
			memcpy( recptr->data, token, strlen(token) + 1);
		}
		else
		{
			if( parseLine( line, "%*s%*[ \t]%*s%*[ \t]%s", token) == 0)
			{
				syslog( LOG_ERR,"Failed to parse value. Line: %s", line);
				continue;
			}
			recptr->data = (char *) malloc( sizeof(char) * (strlen(token) + 1) );
			memcpy( recptr->data, token, strlen(token) + 1);
		}

		memset( line, 0, LINE_MAX);
		record_cnt++;
	}

	zone->record_cnt = record_cnt;
	free(origin);
	fclose(file);

	return 0;
}

char * type_int2str( int type )
{
	char * ret;

	switch( type )
	{
	case A:
		ret = "A";
		break;
	case NS:
		ret = "NS";
		break;
	case MX:
		ret = "MX";
		break;
	case SOA:
		ret = "SOA";
		break;
	case CNAME:
		ret = "CNAME";
		break;
	case PTR:
		ret = "PTR";
		break;
	case TXT:
		ret = "TXT";
		break;
	case AAAA:
		ret = "AAAA";
		break;
	default:
		ret = NULL;
		break;
	}

	return ret;
}

int type_str2int( char * type )
{
	int res;

	if( strcasecmp( type, "A" ) == 0 )
		res = A;
	else if( strcasecmp( type, "NS" ) == 0 )
		res = NS;
	else if( strcasecmp( type, "MX" ) == 0 )
		res = MX;
	else if( strcasecmp( type, "SOA" ) == 0 )
		res = SOA;
	else if( strcasecmp( type, "CNAME" ) == 0 )
		res = CNAME;
	else if( strcasecmp( type, "PTR" ) == 0 )
		res = PTR;
	else if( strcasecmp( type, "TXT" ) == 0 )
		res = TXT;
	else if( strcasecmp( type, "AAAA" ) == 0 )
		res = AAAA;
	else 
		res = -1;

	return res;
}

void postprocess( char * data ){

	int i, j ;
	struct app_data * pdata = (struct app_data *) data;

	for( i=0; i<pdata->zone_cnt; i++)
	{
		for( j=0; j<pdata->zone_list[i]->record_cnt; j++)
		{
			free( pdata->zone_list[i]->rrlist[j]->dname );
			free( pdata->zone_list[i]->rrlist[j]->class );
			if( pdata->zone_list[i]->rrlist[j]->type == SOA )
			{
				free( pdata->zone_list[i]->rrlist[j]->add_data->mname );
				free( pdata->zone_list[i]->rrlist[j]->add_data->rname );
				free( pdata->zone_list[i]->rrlist[j]->add_data );
			}else
				free( pdata->zone_list[i]->rrlist[j]->data );
		}	
	
		if( pdata->zone_list[i]->rrlist != NULL )		
			free( pdata->zone_list[i]->rrlist);

		if( pdata->zone_list[i]->name != NULL )		
			free( pdata->zone_list[i]->name);

		if( pdata->zone_list[i]->ttl != NULL )		
			free( pdata->zone_list[i]->ttl);

		if( pdata->zone_list[i] != NULL )		
			free( pdata->zone_list[i] );
	}

	free( pdata->zone_list );	
	free( pdata );	
}

int name_server( ProcArg * argument  ){
	
	struct ns_header * header;
	struct ns_question * question;
	char * ptr;
	char buf[DOMAIN_LEN];
	int size;
	int record_cnt;
	char * rrptr;
	char * temp;
	int ans_cnt, add_cnt, auth_cnt, rcode, isrecurse;
	struct app_data * pdata;


	header = ( struct ns_header*) argument->proto.udpArg.buffer;
	
	ptr =  argument->proto.udpArg.buffer + sizeof( struct ns_header);

	memset( buf, 0, DOMAIN_LEN);
	size = 0;
	while( *ptr != 0 )
	{
		memcpy( buf+size, ptr+1, *ptr);
		size += *ptr;
		buf[size] = '.';
		size += 1;
		ptr += (*ptr) +1;
	}	

	question = ( struct ns_question *) (argument->proto.udpArg.buffer + sizeof(struct ns_header) + size + 1);
	pdata = ( struct app_data * ) argument->process_data;

        //syslog( LOG_WARNING, "%s queries: info: client %s#%d: query: %s %s %s -", date_str, argument->client_addr, argument->client_port, buf, "IN", type_int2str(ntohs(question->qtype)));
    time_t seconds;
    seconds = time (NULL);

    syslog( LOG_WARNING, "%s %d %s %d udp \"%s IN %s\" %ld", argument->client_addr, argument->client_port, 
			  				     argument->server_addr, argument->server_port,
							     buf, type_int2str(ntohs(question->qtype)), seconds);

	rrptr = search_record( pdata, buf, ntohs(question->qtype), ntohs(question->qclass), &size, &ans_cnt, &auth_cnt, &add_cnt, &rcode, &isrecurse);

	header->ra = recursion;

	if( rrptr == NULL )
	{
		header->qr = ns_s_an;
		header->rcode = rcode ;
		header->ans_count = 0;
		header->auth_count = 0;
		header->add_count = 0;

		if( sendto( argument->proto.udpArg.socket,
					argument->proto.udpArg.buffer,
					argument->proto.udpArg.length,
					0,
					argument->proto.udpArg.socket_addr,
					argument->proto.udpArg.addr_len )==-1)
		{
			perror("sendto");
		}

	}
	else
	{
		if( isrecurse == 1)	
		{
			((struct ns_header*) rrptr)->id = header->id;

			if( sendto( argument->proto.udpArg.socket, 
						rrptr, 
						size, 
						0, 
						argument->proto.udpArg.socket_addr, 
						argument->proto.udpArg.addr_len )==-1)
			{
				perror("sendto");
			}
		}
		else
		{
			if( rcode == ns_r_noerror )
			{	
				header->qr = ns_s_an;
				header->rcode =  ns_r_noerror ;
				header->ans_count = htons(ans_cnt);
				header->auth_count = htons(auth_cnt);
				header->add_count = htons(add_cnt);

				temp = (char *) malloc( (size + argument->proto.udpArg.length) * sizeof(char));
				memcpy( temp, argument->proto.udpArg.buffer, argument->proto.udpArg.length);
				memcpy( temp+argument->proto.udpArg.length, rrptr, size);

				if( sendto( argument->proto.udpArg.socket, 
							temp, 
							argument->proto.udpArg.length+size, 
							0, 
							argument->proto.udpArg.socket_addr, 
							argument->proto.udpArg.addr_len )==-1)
				{
					perror("sendto");
				}
				record_cnt = ans_cnt + auth_cnt + add_cnt;
				free(temp);	
			}
			else
			{
				header->qr = ns_s_an;
				header->rcode = rcode ;
				header->ans_count = 0;
				header->auth_count = 0;
				header->add_count = 0;

				if( sendto( argument->proto.udpArg.socket, 
							argument->proto.udpArg.buffer, 
							argument->proto.udpArg.length, 
							0, 
							argument->proto.udpArg.socket_addr, 
							argument->proto.udpArg.addr_len )==-1)
				{
					perror("sendto");
				}
			}
		}
	}

	return 1;
}

char* compress( char *name)
{
        char * comp;
        char * dotptr;  /* shows first dot */
        char * iterptr; /* current location in str */
        int size, length ;

        comp = NULL;
        iterptr = name;

        size = 0;
        while( (dotptr = strchr(iterptr, '.')) != NULL )
        {

                length = dotptr - iterptr;
                comp = (char *) realloc( comp, size+length+1);
                comp[size] = length;
                memcpy( comp+size+1, iterptr, length);
                size += length + 1;

                if( *(dotptr+1) == '\0' )
                {
                        comp = (char *) realloc( comp, size+2);
                        comp[size] = 0;
                        break;
                }else
                {
                        iterptr = dotptr + 1;
                }
        }
        return comp;
}

char * search_record( struct app_data * pdata, char * name, int qtype, int qclass, int * msg_size, int * ans_cnt, int * auth_cnt, int * add_cnt, int * rcode, int * isrecurse){

	int i;
	int zlength;
	int dlength;
	struct zone * zptr;
	char * results;
	char * dname;
	char * ptr;
	struct ns_rrbody rbody;
	int found;
	int size;
	int total_size;
	char *rname;
	char *rdata;
	char *temp_buf;
	int data_len;
	struct in_addr inp;
	int ret;
	int len;
	struct sockaddr_in6 sa;
	


	if( (qtype!=A) && (qtype!=NS) && (qtype!=CNAME) && (qtype!=SOA) && (qtype!=PTR) && (qtype!=MX) && (qtype!=AAAA))
	{
		*rcode = ns_r_notimpl;
		return NULL;
	}

	for( i=0, zptr=NULL; i< pdata->zone_cnt; i++)
	{
		zlength = strlen( pdata->zone_list[i]->name );
		dlength = strlen( name );

		/* check if zone name is a suffix of domain name */	
		if( strncmp( name+(dlength-zlength), pdata->zone_list[i]->name, strlen(pdata->zone_list[i]->name)) == 0)
		{
			/* I am an authority server for this query */

			zptr = pdata->zone_list[i];
			dname = pdata->zone_list[i]->name;
			break;
		}
	}

	/* recursion desired */
	if( zptr == NULL )
	{
		if( recursion == 1 )
		{
			results = (char *) malloc(sizeof(char) * 1024);
			ret = res_query(name,qclass,qtype,(u_char *)results,1024);
			if( ret == -1 )
			{
				perror("res_query");
				return NULL;	
			}
	
			*isrecurse = 1;	
			*msg_size = ret;
			return results;	
		}
		else
		{
			*rcode = ns_r_noerror;
			return NULL;
		}

	}

	results = NULL;
	total_size = 0;
	*ans_cnt = 0;
	*add_cnt = 0;
	*auth_cnt = 0;
	found = 0;
	for( i=0; i<zptr->record_cnt; i++ )
	{
		if( strcmp( name, zptr->rrlist[i]->dname) == 0)
		{
			if( qtype ==  zptr->rrlist[i]->type)
			{
				rname = compress( dname );
				switch( qtype )
				{
					case PTR:
						rdata = compress( zptr->rrlist[i]->data );
						data_len = strlen(rdata) + 1;
						*ans_cnt += 1;
						*add_cnt += 0;
						*auth_cnt += 0;
						break;
					case CNAME:
						rdata = compress( zptr->rrlist[i]->data );
						data_len = strlen(rdata) + 1;
						*ans_cnt += 1;
						*add_cnt += 0;
						*auth_cnt += 0;
						break;
					case A:
						inet_aton( zptr->rrlist[i]->data, &inp);
						data_len = sizeof(inp);
						rdata = (char *) malloc( data_len * sizeof(char));
						memcpy( rdata, &inp, data_len);
						*ans_cnt += 1;
						*add_cnt += 0;
						*auth_cnt += 0;
						break;
					case AAAA:
						inet_pton(AF_INET6,zptr->rrlist[i]->data, &(sa.sin6_addr));
						data_len = 16;
						rdata = (char *) malloc( data_len * sizeof(char));
						memcpy( rdata, &(sa.sin6_addr), data_len);
						*ans_cnt += 1;
						*add_cnt += 0;
						*auth_cnt += 0;
						break;
					case NS:
						rdata = compress( zptr->rrlist[i]->data );
						data_len = strlen(rdata) + 1;
						*ans_cnt += 1;
						*add_cnt += 0;
						*auth_cnt += 0;
						break;
					case MX:
                                                temp_buf = compress( zptr->rrlist[i]->data );
                                                data_len = strlen(temp_buf) + 1 + sizeof(short int);
                                                rdata = (char *) malloc( data_len );
                                                *((short int *) rdata) = htons(zptr->rrlist[i]->priority);

                                                memcpy( rdata+sizeof(short int), temp_buf, strlen(temp_buf) + 1);

                                                free(temp_buf);
                                                *ans_cnt += 1;
                                                *add_cnt += 0;
                                                *auth_cnt += 0;

						break;
					case SOA:
						temp_buf = compress( zptr->rrlist[i]->add_data->mname );
						len = strlen(temp_buf) + 1;
						rdata = (char *) malloc( len );
						memcpy( rdata, temp_buf, len);
						data_len = len;
						free( temp_buf );
					
						temp_buf = compress( zptr->rrlist[i]->add_data->rname );
						len = strlen(temp_buf) + 1;
						rdata = (char *) realloc( rdata, data_len+len );
						memcpy( rdata+data_len, temp_buf, len);
						data_len += len;
						free( temp_buf );
						
						rdata = (char *) realloc( rdata, data_len+(5 * sizeof(int)) );
						
						memcpy( rdata+data_len, &(zptr->rrlist[i]->add_data->serial), len);
						len = sizeof(int);
						data_len += len;
						memcpy( rdata+data_len, &(zptr->rrlist[i]->add_data->refresh), len);
						data_len += len;
						memcpy( rdata+data_len, &(zptr->rrlist[i]->add_data->retry), len);
						data_len += len;
						memcpy( rdata+data_len, &(zptr->rrlist[i]->add_data->expire), len);
						data_len += len;
						memcpy( rdata+data_len, &(zptr->rrlist[i]->add_data->minimum), len);
						data_len += len;
						
                                                *ans_cnt += 1;
                                                *add_cnt += 0;
                                                *auth_cnt += 0;
						break;
					case TXT:
						break;
					default:
						break;
				}	

				/* compress domain name */	
				rbody.type = htons( zptr->rrlist[i]->type );
				rbody.class = htons(1); /* internet class */
				rbody.ttl = htons(1);
				rbody.data_len = htons(data_len);
			
				size = strlen(rname)+1+ 10 +data_len;
				results = (char *) realloc( results, (total_size + size)*sizeof(char));

				ptr = results + total_size;
				memcpy( ptr, rname, strlen(rname)+1 );
				ptr += strlen(rname)+1;
				memcpy( ptr, &rbody, 10);
				ptr += 10;
				memcpy( ptr, rdata, data_len );

				total_size += size;
				*rcode = ns_r_noerror;
				*isrecurse = 0;	
				free(rdata);
				free(rname);
			}
		}
	}
	*msg_size = total_size;
	
	return results;
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

        while ((ch = getopt(argc, argv, "h:p:c:r:d:")) != -1)
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
                case 'r':
                        rflag = 1;
			recursion = atoi(optarg);
                        break;
                case 'd':
                        pidfile = (char *) malloc(strlen(optarg) + 1);
                        strcpy(pidfile, optarg);
                        break;
                
                default:
                        break;
                }

        }
        if( pflag == 0 || hflag == 0 || cflag==0)
        {
                printf( "usage: kovan_dns [-d pidfile] -h host -p port -c config [-r recurse] \n");
                return 1;
        }

        gs_start( UDP, pidfile, host, port, name_server, preprocessor, postprocess);

        free(host);
        free(port);
        free(configfile);
        free(pidfile);

	return 0;
}
