#ifndef TRAFFIC_H
#define TRAFFIC_H

#include<string.h>      //memset
#include<netinet/ip_icmp.h>     //Provides declarations for icmp header
#include<netinet/udp.h> //Provides declarations for udp header
#include<netinet/tcp.h> //Provides declarations for tcp header
#include<netinet/ip.h>  //Provides declarations for ip header
#include<arpa/inet.h>

#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

void trafficmeter(char iface[], int sampletime);
void livetrafficmeter(char iface[], int mode);
void sniffing_process(char*);
void  ProcessPacket(char* , int, void*);
char* print_source_ip(char*, int);
void print_ip_header(char* , int);
void print_tcp_packet(char* , int);
void print_udp_packet(char * , int);
void print_icmp_packet(char* , int);
void PrintData (char* , int);
void* create_shared_memory(size_t);

#endif