#include "common.h"
#include "ifinfo.h"
#include "misc.h"
#include "traffic.h"

int sock_raw;
int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,j,k, num=0;
struct sockaddr_in source,dest;
int saddr_size , data_size;
struct sockaddr saddr;
struct in_addr in;
FILE *logfile;

void trafficmeter(char iface[], int sampletime)
{
	/* received bytes packets errs drop fifo frame compressed multicast */
	/* transmitted bytes packets errs drop fifo colls carrier compressed */
	uint64_t rx, tx, rxp, txp;
	IFINFO firstinfo;
	int i, len;
	char buffer[256];
	/* less than 2 seconds doesn't produce good results */
	if (sampletime<2) {
		printf("Error: Time for sampling too short.\n");
		exit(EXIT_FAILURE);
	}

	/* read interface info and get values to the first list */
	if (!getifinfo(iface)) {
		printf("Error: Interface \"%s\" not available, exiting.\n", iface);
		exit(EXIT_FAILURE);
	}
	firstinfo.rx = ifinfo.rx;
	firstinfo.tx = ifinfo.tx;
	firstinfo.rxp = ifinfo.rxp;
	firstinfo.txp = ifinfo.txp;

	/* wait sampletime and print some nice dots so that the user thinks
	something is done :) */
	snprintf(buffer, 256, "Sampling %s (%d seconds average)", iface,sampletime);
	printf("%s", buffer);
	fflush(stdout);
	sleep(sampletime/3);
	printf(".");
	fflush(stdout);
	sleep(sampletime/3);
	printf(".");
	fflush(stdout);
	sleep(sampletime/3);
	printf(".");
	fflush(stdout);
	if ((sampletime/3)*3!=sampletime) {
		sleep(sampletime-((sampletime/3)*3));
	}

	len=strlen(buffer)+3;

	for (i=0;i<len;i++) {
		printf("\b \b");
	}

	/* read those values again... */
	if (!getifinfo(iface)) {
		printf("Error: Interface \"%s\" not available, exiting.\n", iface);
		exit(EXIT_FAILURE);
	}

	/* calculate traffic and packets seen between updates */
	rx = rintf(countercalc(firstinfo.rx, ifinfo.rx)/(float)1024);
	tx = rintf(countercalc(firstinfo.tx, ifinfo.tx)/(float)1024);
	rxp = countercalc(firstinfo.rxp, ifinfo.rxp);
	txp = countercalc(firstinfo.txp, ifinfo.txp);

	/* show the difference in a readable format */
	printf("%"PRIu64" packets sampled in %d seconds\n", rxp+txp, sampletime);
	printf("Traffic average for %s\n", iface);
	printf("\n      rx     %s         %5"PRIu64" packets/s\n", getrate(0, rx, sampletime, 15), (uint64_t)(rxp/sampletime));
	printf("      tx     %s         %5"PRIu64" packets/s\n\n", getrate(0, tx, sampletime, 15), (uint64_t)(txp/sampletime));

}

void livetrafficmeter(char iface[32], int mode)
{
	/* received bytes packets errs drop fifo frame compressed multicast */
	/* transmitted bytes packets errs drop fifo colls carrier compressed */
	uint64_t rx, tx, rxp, txp, rxpc, txpc, timespent;
	uint64_t rxtotal, txtotal, rxptotal, txptotal;
	uint64_t rxpmin, txpmin, rxpmax, txpmax;
	float rxmin, txmin, rxmax, txmax, rxc, txc;
	int i, len=0;
	char buffer[1024], buffer2[256];
	char buffer3[512]; // buffer for the number of protocol type
	IFINFO previnfo;

	printf("Monitoring %s...    (press CTRL-C to stop)\n\n", iface);
	if (cfg.ostyle != 4) {
		printf("   getting traffic...");
		len=21; /* length of previous print */
		fflush(stdout);
	}

	/* enable signal trap */
	intsignal = 0;
	if (signal(SIGINT, sighandler) == SIG_ERR) {
		perror("signal");
		exit(EXIT_FAILURE);
	}

	/* set some defaults */
	rxtotal=txtotal=rxptotal=txptotal=rxpmax=txpmax=0;
	rxpmin=txpmin=FP64;
	rxmax=txmax=0.0;
	rxmin=txmin=-1.0;

	timespent = (uint64_t)time(NULL);

	/* read /proc/net/dev and get values to the first list */
	if (!getifinfo(iface)) {
		printf("Error: Interface \"%s\" not available, exiting.\n", iface);
		exit(EXIT_FAILURE);
	}

	/* connect to socket */
	pid_t pid;
	void* shmem = create_shared_memory(256);

	logfile=fopen("log.txt","w");

	pid = fork();
        //printf("pid = [%d]\n", getpid());

        if(pid <0){
                printf("fork Error...: return is [%d] \n", pid);
                perror("fork error : ");
                exit(0);
        }
        //Create a raw socket that shall sniff
        else if(pid == 0){

                /* socket open */

                char* buffer = (char *)malloc(63556);

                sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);

                if(sock_raw < 0)
                {
                        perror("Socket Error\n");
                }
                saddr_size = sizeof saddr;

                pid_t ppid = getppid();

                do{

                        //Receive a packet
                        data_size = recvfrom(sock_raw , buffer , 63556 , 0 , &saddr , &saddr_size);

                        if( data_size <0 )
                        {
                                perror("Recvfrom error , failed to get packets\n");
                        }

                        //Now process the packet
                        ProcessPacket(buffer , data_size, shmem);

                }while(kill(ppid, 0) == 0);

                close(sock_raw);

		// 프로세스 모두 종료한 뒤에 자식 종료 !! 여기 실행됨!!  확인함!!!
 } else {

                if(logfile==NULL){
                        printf("Unable to create file.");
                        fflush(stdout);
                }

	/* loop until user gets bored */
	while (intsignal==0) {

		/* wait 2 seconds for more traffic */
		sleep(LIVETIME);

		/* break loop without calculations because sleep was probably interrupted */
		if (intsignal) {
			break;
		}

		/* use values from previous loop if this isn't the first time */
		previnfo.rx = ifinfo.rx;
		previnfo.tx = ifinfo.tx;
		previnfo.rxp = ifinfo.rxp;
		previnfo.txp = ifinfo.txp;

		/* read those values again... */
		if (!getifinfo(iface)) {
			printf("Error: Interface \"%s\" not available, exiting.\n", iface);
			exit(EXIT_FAILURE);
		}

		/* calculate traffic and packets seen between updates */
		rx = countercalc(previnfo.rx, ifinfo.rx);
		tx = countercalc(previnfo.tx, ifinfo.tx);
		rxp = countercalc(previnfo.rxp, ifinfo.rxp);
		txp = countercalc(previnfo.txp, ifinfo.txp);

		/* update totals */
		rxtotal += rx;
		txtotal += tx;
		rxptotal += rxp;
		txptotal += txp;

		rxc = rintf(rx/(float)1024);
		txc = rintf(tx/(float)1024);
		rxpc = rxp/LIVETIME;
		txpc = txp/LIVETIME;

		/* update min & max */
		if ((rxmin==-1.0) || (rxmin>rxc)) {
			rxmin = rxc;
		}
		if ((txmin==-1.0) || (txmin>txc)) {
			txmin = txc;
		}
		if (rxmax<rxc) {
			rxmax = rxc;
		}
		if (txmax<txc) {
			txmax = txc;
		}

		if ((rxpmin==FP64) || (rxpmin>rxpc)) {
			rxpmin = rxpc;
		}
		if ((txpmin==FP64) || (txpmin>txpc)) {
			txpmin = txpc;
		}
		if (rxpmax<rxpc) {
			rxpmax = rxpc;
		}
		if (txpmax<txpc) {
			txpmax = txpc;
		}

		/* show the difference in a readable format */
		if (cfg.ostyle != 0) {
			if (mode == 0) {
				snprintf(buffer, 128, "\r    rx: %s %5"PRIu64" p/s", getrate(0, rxc, LIVETIME, 15), (uint64_t)rxpc);
				snprintf(buffer2, 128, "          tx: %s %5"PRIu64" p/s", getrate(0, txc, LIVETIME, 15), (uint64_t)txpc);

			} else {
				snprintf(buffer, 128, "\r    rx: %s   %s", getrate(0, rxc, LIVETIME, 13), getvalue(0, rintf(rxtotal/(float)1024), 1, 1));
				snprintf(buffer2, 128, "             tx: %s   %s", getrate(0, txc, LIVETIME, 13), getvalue(0, rintf(txtotal/(float)1024), 1, 1));
			}
		} else {
			if (mode == 0) {
				snprintf(buffer, 128, "\r    rx: %s %3"PRIu64" p/s", getrate(0, rxc, LIVETIME, 12), (uint64_t)rxpc);
				snprintf(buffer2, 128, "      tx: %s %3"PRIu64" p/s", getrate(0, txc, LIVETIME, 12), (uint64_t)txpc);
			} else {
				snprintf(buffer, 128, "\r    rx: %s  %s", getrate(0, rxc, LIVETIME, 12), getvalue(0, rintf(rxtotal/(float)1024), 1, 1));
				snprintf(buffer2, 128, "       tx: %s  %s", getrate(0, txc, LIVETIME, 12), getvalue(0, rintf(txtotal/(float)1024), 1, 1));
			}
		}

		snprintf(buffer3, 256, "%s", shmem);

		strncat(buffer, buffer2, 128);
		strncat(buffer, buffer3, 256);

		if (len>1 && cfg.ostyle!=4) {
			if (debug) {
				printf("\nlinelen: %d\n", len);
			} else {
				for (i=0;i<len;i++) {
					printf("\b \b");
				}
				fflush(stdout);
			}
		}

		if (cfg.ostyle!=4) {
			printf("%s", buffer);
			fflush(stdout);
		} else {
			printf("%s", buffer);
			fflush(stdout);
		}
		len=strlen(buffer);

	} // while end

	timespent = (uint64_t)time(NULL) - timespent;

	printf("\n\n");

	/* print some statistics if enough time did pass */
	if (timespent>10) {

		printf("\n %s  /  traffic statistics\n\n", iface);

		printf("                           rx         |       tx\n");
		printf("--------------------------------------+------------------\n");
		printf("  bytes                %s", getvalue(0, rintf(rxtotal/(float)1024), 13, 1));
		printf("  |   %s", getvalue(0, rintf(txtotal/(float)1024), 13, 1));
		printf("\n");
		printf("--------------------------------------+------------------\n");
		printf("          max          %s", getrate(0, rxmax, LIVETIME, 13));
		printf("  |   %s\n", getrate(0, txmax, LIVETIME, 13));
		printf("      average          %s", getrate(0, rintf(rxtotal/(float)1024), timespent, 13));
		printf("  |   %s\n", getrate(0, rintf(txtotal/(float)1024), timespent, 13));
		printf("          min          %s", getrate(0, rxmin, LIVETIME, 13));
		printf("  |   %s\n", getrate(0, txmin, LIVETIME, 13));
		printf("--------------------------------------+------------------\n");
		printf("  packets               %12"PRIu64"  |    %12"PRIu64"\n", rxptotal, txptotal);
		printf("--------------------------------------+------------------\n");
		printf("          max          %9"PRIu64" p/s  |   %9"PRIu64" p/s\n", rxpmax, txpmax);
		printf("      average          %9"PRIu64" p/s  |   %9"PRIu64" p/s\n", rxptotal/timespent, txptotal/timespent);
		printf("          min          %9"PRIu64" p/s  |   %9"PRIu64" p/s\n", rxpmin, txpmin);
		printf("--------------------------------------+------------------\n");

		if (timespent<=60) {
			printf("  time             %9"PRIu64" seconds\n", timespent);
		} else {
			printf("  time               %7.2f minutes\n", timespent/(float)60);
		}

		printf("\n");
	}

    } //end of parent process

}

void* create_shared_memory(size_t size){
         int protection = PROT_READ | PROT_WRITE;

         // The buffer will be shared (meaning other processes can access it), but
         // anonymous (meaning third-party processes cannot obtain an address for it),
         // so only this process and its children will be able to use it:

         int visibility = MAP_SHARED | MAP_ANONYMOUS;

         // The remaining parameters to `mmap()` are not important for this use case,
         // but the manpage for `mmap` explains their purpose.
         return mmap(NULL, size, protection, visibility, -1, 0);
}

void ProcessPacket(char* buffer, int size, void* shmem)
{

        //Get the IP Header part of this packet
        struct iphdr *iph = (struct iphdr*)buffer;
        ++total;

        switch (iph->protocol) //Check the Protocol and do accordingly...
        {
                case 1:  //ICMP Protocol
                        ++icmp;
                        print_icmp_packet(buffer,size);
                        break;

                case 2:  //IGMP Protocol
                        ++igmp;
                        break;
                case 6:  //TCP Protocol
                        ++tcp;
                        print_tcp_packet(buffer , size);
                        break;

                case 17: //UDP Protocol
                        ++udp;
                        print_udp_packet(buffer , size);
                        break;

                default: //Some Other Protocol like ARP etc.
                        ++others;
                        break;
        }

        char message[128];
	sprintf(message,  "\n    TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\n    Received a packet from %s",tcp,udp,icmp,igmp,others,total, print_source_ip(buffer, size));
        memcpy(shmem, message, sizeof(message));

}

char* print_source_ip(char * buffer, int size){

        unsigned short iphdrlen;

        struct iphdr *iph = (struct iphdr *)buffer;
        iphdrlen = iph->ihl*4;

        memset(&source, 0, sizeof(source));
        source.sin_addr.s_addr = iph->saddr;

        char* ip_addr = inet_ntoa(source.sin_addr);

        return ip_addr;
}


void print_ip_header(char* Buffer, int Size)
{
        unsigned short iphdrlen;

        struct iphdr *iph = (struct iphdr *)Buffer;
        iphdrlen =iph->ihl*4;

        memset(&source, 0, sizeof(source));
        source.sin_addr.s_addr = iph->saddr;

        memset(&dest, 0, sizeof(dest));
        dest.sin_addr.s_addr = iph->daddr;

        fprintf(logfile,"\n");
        fprintf(logfile,"IP Header\n");
        fprintf(logfile,"   |-IP Version        : %d\n",(unsigned int)iph->version);
        fprintf(logfile,"   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
        fprintf(logfile,"   |-Type Of Service   : %d\n",(unsigned int)iph->tos);
        fprintf(logfile,"   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iph->tot_len));
        fprintf(logfile,"   |-Identification    : %d\n",ntohs(iph->id));
        //fprintf(logfile,"   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
        //fprintf(logfile,"   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
        //fprintf(logfile,"   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
        fprintf(logfile,"   |-TTL      : %d\n",(unsigned int)iph->ttl);
        fprintf(logfile,"   |-Protocol : %d\n",(unsigned int)iph->protocol);
        fprintf(logfile,"   |-Checksum : %d\n",ntohs(iph->check));
        fprintf(logfile,"   |-Source IP        : %s\n",inet_ntoa(source.sin_addr));
  	fprintf(logfile,"   |-Destination IP   : %s\n",inet_ntoa(dest.sin_addr));
}

void print_tcp_packet(char* Buffer, int Size)
{
        unsigned short iphdrlen;

        struct iphdr *iph = (struct iphdr *)Buffer;
        iphdrlen = iph->ihl*4;

        struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen);

        fprintf(logfile,"\n\n***********************TCP Packet*************************\n");

        print_ip_header(Buffer,Size);

        fprintf(logfile,"\n");
        fprintf(logfile,"TCP Header\n");
        fprintf(logfile,"   |-Source Port      : %u\n",ntohs(tcph->source));
        fprintf(logfile,"   |-Destination Port : %u\n",ntohs(tcph->dest));
        fprintf(logfile,"   |-Sequence Number    : %u\n",ntohl(tcph->seq));
        fprintf(logfile,"   |-Acknowledge Number : %u\n",ntohl(tcph->ack_seq));
        fprintf(logfile,"   |-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcph->doff,(unsigned int)tcph->doff*4);
  	//fprintf(logfile,"   |-ECN Flag : %d\n",(unsigned int)tcph->ece);
        fprintf(logfile,"   |-Urgent Flag          : %d\n",(unsigned int)tcph->urg);
        fprintf(logfile,"   |-Acknowledgement Flag : %d\n",(unsigned int)tcph->ack);
        fprintf(logfile,"   |-Push Flag            : %d\n",(unsigned int)tcph->psh);
        fprintf(logfile,"   |-Reset Flag           : %d\n",(unsigned int)tcph->rst);
        fprintf(logfile,"   |-Synchronise Flag     : %d\n",(unsigned int)tcph->syn);
        fprintf(logfile,"   |-Finish Flag          : %d\n",(unsigned int)tcph->fin);
        fprintf(logfile,"   |-Window         : %d\n",ntohs(tcph->window));
        fprintf(logfile,"   |-Checksum       : %d\n",ntohs(tcph->check));
        fprintf(logfile,"   |-Urgent Pointer : %d\n",tcph->urg_ptr);
        fprintf(logfile,"\n");
        fprintf(logfile,"                        DATA Dump                         ");
        fprintf(logfile,"\n");

        fprintf(logfile,"IP Header\n");
        PrintData(Buffer,iphdrlen);

        fprintf(logfile,"TCP Header\n");
        PrintData(Buffer+iphdrlen,tcph->doff*4);

        fprintf(logfile,"Data Payload\n");
        PrintData(Buffer + iphdrlen + tcph->doff*4 , (Size - tcph->doff*4-iph->ihl*4) );

        fprintf(logfile,"\n###########################################################");
}

void print_udp_packet(char *Buffer , int Size)
{

        unsigned short iphdrlen;

        struct iphdr *iph = (struct iphdr *)Buffer;
        iphdrlen = iph->ihl*4;

        struct udphdr *udph = (struct udphdr*)(Buffer + iphdrlen);

        fprintf(logfile,"\n\n***********************UDP Packet*************************\n");

        print_ip_header(Buffer,Size);

        fprintf(logfile,"\nUDP Header\n");
        fprintf(logfile,"   |-Source Port      : %d\n" , ntohs(udph->source));
        fprintf(logfile,"   |-Destination Port : %d\n" , ntohs(udph->dest));
        fprintf(logfile,"   |-UDP Length       : %d\n" , ntohs(udph->len));
        fprintf(logfile,"   |-UDP Checksum     : %d\n" , ntohs(udph->check));
	fprintf(logfile,"\n");
        fprintf(logfile,"IP Header\n");
        PrintData(Buffer , iphdrlen);

        fprintf(logfile,"UDP Header\n");
        PrintData(Buffer+iphdrlen , sizeof udph);

        fprintf(logfile,"Data Payload\n");
        PrintData(Buffer + iphdrlen + sizeof udph ,( Size - sizeof udph - iph->ihl * 4 ));

        fprintf(logfile,"\n###########################################################");
}

void print_icmp_packet(char* Buffer , int Size)
{
        unsigned short iphdrlen;

        struct iphdr *iph = (struct iphdr *)Buffer;
        iphdrlen = iph->ihl*4;

        struct icmphdr *icmph = (struct icmphdr *)(Buffer + iphdrlen);

        fprintf(logfile,"\n\n***********************ICMP Packet*************************\n");

        print_ip_header(Buffer , Size);

        fprintf(logfile,"\n");

        fprintf(logfile,"ICMP Header\n");
        fprintf(logfile,"   |-Type : %d",(unsigned int)(icmph->type));

        if((unsigned int)(icmph->type) == 11)
                fprintf(logfile,"  (TTL Expired)\n");
        else if((unsigned int)(icmph->type) == ICMP_ECHOREPLY)
                fprintf(logfile,"  (ICMP Echo Reply)\n");
        fprintf(logfile,"   |-Code : %d\n",(unsigned int)(icmph->code));
        fprintf(logfile,"   |-Checksum : %d\n",ntohs(icmph->checksum));
        //fprintf(logfile,"   |-ID       : %d\n",ntohs(icmph->id));
        //fprintf(logfile,"   |-Sequence : %d\n",ntohs(icmph->sequence));
        fprintf(logfile,"\n");

        fprintf(logfile,"IP Header\n");
        PrintData(Buffer,iphdrlen);

        fprintf(logfile,"UDP Header\n");
	PrintData(Buffer + iphdrlen , sizeof icmph);

        fprintf(logfile,"Data Payload\n");
        PrintData(Buffer + iphdrlen + sizeof icmph , (Size - sizeof icmph - iph->ihl * 4));

        fprintf(logfile,"\n###########################################################");
}

void PrintData (char* data , int Size)
{
	int i;

        for(i=0 ; i < Size ; i++)
        {
                if( i!=0 && i%16==0)   //if one line of hex printing is complete...
                {
                        fprintf(logfile,"         ");
                        for(j=i-16 ; j<i ; j++)
                        {
                                if(data[j]>=32 && data[j]<=128)
                                        fprintf(logfile,"%c",(unsigned char)data[j]); //if its a number or alphabet

                                else fprintf(logfile,"."); //otherwise print a dot
                        }
           fprintf(logfile,"\n");
                }

                if(i%16==0) fprintf(logfile,"   ");
                        fprintf(logfile," %02X",(unsigned int)data[i]);

                if( i==Size-1)  //print the last spaces
                {
                        for(j=0;j<15-i%16;j++) fprintf(logfile,"   "); //extra spaces

                        fprintf(logfile,"         ");

                        for(j=i-i%16 ; j<=i ; j++)
                        {
                                if(data[j]>=32 && data[j]<=128) fprintf(logfile,"%c",(unsigned char)data[j]);
                                else fprintf(logfile,".");
                        }
                        fprintf(logfile,"\n");
                }
        }
}