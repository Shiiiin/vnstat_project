/*

/*
int sock_raw;
int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j;
struct sockaddr_in source,dest;
int saddr_size , data_size;
struct sockaddr saddr;
struct in_addr in;
FILE *logfile;
*/

void sniffing_init(char* buffer)
{

/*
	pid_t pid;

	void* shmem = create_shared_memory(256);

	pid = fork();
	printf("pid = [%d]\n", getpid());

	if(pid <0){
		printf("fork Error...: return is [%d] \n", pid);
		perror("fork error : ");
		exit(0);
	}
	//Create a raw socket that shall sniff
	else if(pid == 0){

		
		/* shared memory */
		/*
		int shmid;
		int i;
		SHM_INFOS *shm_info = NULL;
		void *shared_memory = (void* )0;

		shmid = shmget((key_t)3836, sizeof(SHM_INFOS)*1024, 0666|IPC_CREAT);

		if (shmid == -1)
		{
			perror("shmget failed : ");
			exit(0);
		}

		shared_memory = shmat(shmid, (void* )0, 0);
		if (shared_memory == (void *)-1)
		{
			perror("shmat failed : ");
			exit(0);
		}

		shm_info = (SHM_INFOS *)shared_memory;
		*/

		/* socket open */

		char* buffer = (char *)malloc(63556);

		sock_raw = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);

		if(sock_raw < 0)
		{
			perror("Socket Error\n");
		}
		saddr_size = sizeof saddr;

		pid_t ppid = getppid();
	
		printf("parent process in child process : %d\n", ppid);

		i = 0;
		do{
			
			//Receive a packet
			data_size = recvfrom(sock_raw , buffer , 63556 , 0 , &saddr , &saddr_size);
			
			if( data_size <0 )
			{
 				perror("Recvfrom error , failed to get packets\n");
			}
	
			//Now process the packet
			ProcessPacket(buffer , data_size, shmem); 

			i++;

		}while(kill(ppid, 0) == 0);

		i = 0;
		close(sock_raw);
		printf("Finished\n");
	} else {

		logfile=fopen("log.txt","w");
		if(logfile==NULL){
			printf("Unable to create file.");
			fflush(stdout);
		}
	}

*/
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
			//print_icmp_packet(buffer,size);
			break;
		
		case 2:  //IGMP Protocol
			++igmp;
			break;
		
		case 6:  //TCP Protocol
			++tcp;
			//print_tcp_packet(buffer , size);
			break;
		
		case 17: //UDP Protocol
			++udp;
			//print_udp_packet(buffer , size);
			break;
		
		default: //Some Other Protocol like ARP etc.
			++others;
			break;
	}
	//snprintf(shm_info.str_ip, sizeof(shm_info.str_ip), "\n    TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\n    Received a packet from %s",
															 tcp,udp,icmp,igmp,others,total, print_source_ip(buffer, size));

	char* message =  "\n    TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\n    Received a packet from %s", tcp,udp,icmp,igmp,others,total, print_source_ip(buffer, size));	

	memcpy(shmem, message, sizeof(message));

	printf("Child wrote: %s\n", shmem);

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
	//fprintf(logfile,"   |-CWR Flag : %d\n",(unsigned int)tcph->cwr);
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
*/
