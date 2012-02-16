#define APP_NAME	"kovan_dump"

#include <pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <time.h>
#include <unistd.h>

void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void
print_app_usage(void);

/*
 * print help text
 */
void
print_app_usage(void)
{

	printf("Usage: %s [-i interface] [-n packet_number] [-w filename] [-f capture_filter]\n", APP_NAME);
	printf("\n");
	printf("Options:\n");
	printf("    interface      Listen on <interface> for packets.\n");
	printf("    packet_number  Capture <packet_number> packets.\n");
	printf("    filename       Write pcapt to <filename> file.\n");
	printf("    capture_filter Use <capture_filter> as pcap filter string.\n");
	printf("\n");

return;
}

/*
 * dissect/print packet
 */
void
got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{

	/* declare pointers to packet headers */
	const struct ether_header *eth_hd;  	/* The ethernet header [1] */
	const struct ip *ip_hd;              		/* The IP header */
	const struct ip6_hdr *ip6_hd;              		/* The IP header */
	const struct tcphdr *tcp_hd = NULL;
	const struct udphdr *udp_hd = NULL;
	int size_ip;
    time_t seconds;

	char src_addr[INET6_ADDRSTRLEN];
	char dst_addr[INET6_ADDRSTRLEN];
	char proto[16];

	/* define ethernet header */
	eth_hd = (struct ether_header*)(packet);

	if( eth_hd->ether_type == 4 ){

		/* define/compute ip header offset */
		ip_hd = (struct ip*)(packet + ETHER_HDR_LEN);
		size_ip = ip_hd->ip_hl * 4;
		if (size_ip < 20) {
			printf("   * Invalid IP header length: %u bytes\n", size_ip);
			return;
		}

		inet_ntop(AF_INET, &(ip_hd->ip_src) , src_addr, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(ip_hd->ip_dst) , dst_addr, INET_ADDRSTRLEN);

		/* determine protocol */	
		switch(ip_hd->ip_p) {
			case IPPROTO_TCP:
				memcpy( proto, "tcp", 4);
				tcp_hd = (struct tcphdr *)(packet + ETHER_HDR_LEN +sizeof(struct ip));
				break;
			case IPPROTO_UDP:
				memcpy( proto, "udp", 4);
				udp_hd = (struct udphdr *)(packet + ETHER_HDR_LEN +sizeof(struct ip));
				return;
			case IPPROTO_ICMP:
				memcpy( proto, "icmp", 5);
				return;
			case IPPROTO_IP:
				memcpy( proto, "ip", 3);
				return;
			default:
				memcpy( proto, "unknown", 8);
				return;
		}

	}else{

		/* package size check */

		/* define/compute ip header offset */
		ip6_hd = (struct ip6_hdr*)(packet + ETHER_HDR_LEN);
          
		inet_ntop(AF_INET6, &(ip6_hd->ip6_src) , src_addr, INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &(ip6_hd->ip6_dst) , dst_addr, INET6_ADDRSTRLEN);

		/* determine protocol */	
		switch(ip6_hd->ip6_nxt) {
			case IPPROTO_TCP:
				memcpy( proto, "tcp", 4);
				tcp_hd = (struct tcphdr *)(packet + ETHER_HDR_LEN +sizeof(struct ip6_hdr));
				break;
			case IPPROTO_UDP:
				memcpy( proto, "udp", 4);
				udp_hd = (struct udphdr *)(packet + ETHER_HDR_LEN +sizeof(struct ip6_hdr));
				break;
			case IPPROTO_ICMPV6:
				memcpy( proto, "icmp6", 6);
				break;
			case IPPROTO_IPV6:
				memcpy( proto, "ip6", 4);
				break;
			default:
				memcpy( proto, "unknown", 8);
				break;
		}
	}	

	int sport = -1;
	int dport = -1;

	if( tcp_hd != NULL ){
		dport = ntohs(tcp_hd->th_dport);
		sport = ntohs(tcp_hd->th_sport);
	}else if( udp_hd != NULL){
		dport = ntohs(udp_hd->uh_dport);
		sport = ntohs(udp_hd->uh_sport);
	}

    seconds = time (NULL);
	printf("blackhole: %s %d %s %d %s blackhole_flow %ld\n", src_addr, sport, dst_addr, dport, proto, seconds);

	return;
}

int main(int argc, char **argv)
{

	char *dev = NULL;			/* capture device name */
	char errbuf[PCAP_ERRBUF_SIZE];		/* error buffer */
	pcap_t *handle;				/* packet capture handle */

	char * filter_exp = "";		/* filter expression [3] */
	char * outputfile = "";		/* filter expression [3] */
	struct bpf_program fp;		/* compiled filter program (expression) */
	int num_packets = -1;		/* number of packets to capture */
	int err = 0;
	char ch;
	char fflag = 0;
	char iflag = 0;
	char wflag = 0;

	while ((ch = getopt(argc, argv, "i:n:w:f:h")) != -1) {
    	switch (ch) {
    	case 'i':
				dev = (char *) malloc( strlen(optarg)+1);
				memcpy( dev, optarg, strlen(optarg)+1 );
				iflag = 1;
    	        break;
    	case 'n':
				num_packets = atoi(optarg);
    	        break;
    	case 'f':
				filter_exp = (char *) malloc( strlen(optarg)+1);
				memcpy( filter_exp, optarg, strlen(optarg)+1 );
				fflag = 1;
    	        break;
	case 'w':
				outputfile = (char *) malloc( strlen(optarg)+1);
				memcpy( outputfile, optarg, strlen(optarg)+1 );
				wflag = 1;
		break;
    	case 'h':
    	default:
    	        print_app_usage();
				if( dev != NULL)
            		free(dev);
        		if( fflag)
            		free(filter_exp);
				return 1;
    	}
    }

	for(; optind < argc; optind++){
  		printf("Invalid argument: %s\n", argv[optind]);
		err = 1;
	}

	if( !iflag || !fflag ){
		err = 1;
	}

	if( err == 1){
		print_app_usage();
		if( dev != NULL)
			free(dev);
		if( fflag)
			free(filter_exp);
		return 0;
	 }
	
	if( dev == NULL){
		/* find a capture device if not specified on command-line */
		dev = pcap_lookupdev(errbuf);
		if (dev == NULL) {
			fprintf(stderr, "Couldn't find default device: %s\n",
			    errbuf);
			exit(EXIT_FAILURE);
		}
	}
	
	/* open capture device */
	handle = pcap_open_live(dev, ETHER_MAX_LEN, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		exit(EXIT_FAILURE);
	}

	/* make sure we're capturing on an Ethernet device [2] */
	if (pcap_datalink(handle) != DLT_EN10MB) {
		fprintf(stderr, "%s is not an Ethernet\n", dev);
		exit(EXIT_FAILURE);
	}

	/* compile the filter expression */
	if (pcap_compile(handle, &fp, filter_exp, 0, 0) == -1) {
		fprintf(stderr, "Couldn't parse filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	/* apply the compiled filter */
	if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n",
		    filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	pcap_dumper_t *pd;       /* pointer to the dump file */

	if( wflag ){

	        if ((pd = pcap_dump_open(handle,outputfile)) == NULL) {
                	fprintf(stderr,
                        "Error opening savefile \"%s\" for writing: %s\n",
                        outputfile, pcap_geterr(handle));
                	exit(7);
		}
		pcap_loop(handle, num_packets,  &pcap_dump, (u_char *)pd);
	        pcap_dump_close(pd);



	}else{

		/* now we can set our callback function */
		pcap_loop(handle, num_packets, got_packet, NULL);

	}
	/* cleanup */
	pcap_freecode(&fp);
	pcap_close(handle);

	printf("\nCapture complete.\n");
	free(dev);
	free(filter_exp);

	return 0;
}


