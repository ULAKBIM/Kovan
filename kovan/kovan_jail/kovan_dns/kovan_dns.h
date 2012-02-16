struct ns_header
{
	unsigned short id;   
	unsigned char rd:1;
	unsigned char tc:1;
	unsigned char aa:1;
	unsigned char opcode:4;
	unsigned char qr:1;
	unsigned char rcode:4;
	unsigned char zero:3;
	unsigned char ra:1;
	unsigned short q_count;
	unsigned short ans_count;
	unsigned short auth_count;
	unsigned short add_count;
};

struct ns_question
{
	unsigned short qtype;
	unsigned short qclass;
};

struct ns_rrbody
{
	unsigned short type;
	unsigned short class;
	unsigned int ttl;
	unsigned short data_len;
};

struct rrecord{
        char * dname;
        char * class;
        int type;
        char * data;
	int short priority;
	struct soa_data * add_data;
};

struct soa_data{
	char *mname;
	char *rname;
	int serial;
	int refresh;
	int retry;
	int expire;
	int minimum;
};

struct zone{
        char * name;
        char * ttl;
	int record_cnt;
        struct rrecord **rrlist;
};

struct app_data {
        struct zone ** zone_list;
        int zone_cnt;
};



enum{
	A = 1,
	NS = 2,
	CNAME = 5,
	SOA = 6,
	PTR = 12,
	MX = 15,
	TXT = 16,
	AAAA= 28
} qType;


int read_config( char * fname );
int read_zone( char * fname, struct zone * zone);
char * search_record( struct app_data *, char * name, int qType, int qClass, int * size, int * ans_count, int * auth_cnt, int * add_cnt, int * rcode, int * isrecursive );
 
int type_str2int( char * type );

