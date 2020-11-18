/*    
Asha Wright
Simple udp server
*/

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>

#define BUFLEN 512  //Max length of buffer

struct dchp_pkt {
	unsigned long long int siaddr;         // Server IP address
	unsigned long long int yiaddr;         // Your IP address
	unsigned int tran_ID;        // Transaction ID
	unsigned short int lifetime; // Lease time of the IP address
};

void print(struct dchp_pkt packet);


void die(char *s)
{    
	perror(s);    
	exit(1);
}

int main(int argc, char *argv[])
{    
	struct dchp_pkt packet;
	struct sockaddr_in si_me, si_other;    
	int sockfd, i, slen = sizeof(si_other) , recv_len;    
	char buf[BUFLEN];    

	if(argc < 2)
	{
		fprintf(stderr, "To run: %s port \n", argv[0]);
		exit(1);
	}

	int PORT = atoi(argv[1]);

	//create a UDP socket    
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)    
	{        
		die("socket");    
	}    

	// zero out the structure    
	memset((char *) &si_me, 0, sizeof(si_me));    
	si_me.sin_family = AF_INET;    
	si_me.sin_port = htons(PORT);    
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);    

	//bind socket to port    
	if( bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)    
	{        
		die("bind");    
	}    

	//get network address from user
	int IP1, IP2, IP3, IP4;
    	printf("Enter network address in the form of \"0.0.0.0\": \n");
	scanf("%i.%i.%i.%i", &IP1, &IP2, &IP3, &IP4);

	//get subnet part from user
	int subnet_part;
        printf("Enter subnet part in CIDR notation (1 - 32): \n");
        scanf("%d", &subnet_part);
	
	//calculate bits borrowed
	int bits = 32 - subnet_part;

	//find # of usable hosts
	int host = pow(2, bits) - 2;

	//concate into IP format
	char s1[20];
	char s2[20];
	char s3[20];
	char s4[20];

	sprintf(s1, "%d", IP1);
	sprintf(s2, "%d", IP2);
	sprintf(s3, "%d", IP3);
	sprintf(s4, "%d", IP4);

	strcat(s1, ".");
	strcat(s1, s2);
	strcat(s1, ".");
	strcat(s1, s3);
	strcat(s1, ".");
	strcat(s1, s4);

	printf("%s\n", s1);

	//convert to ip address format
	unsigned long long int addr = inet_addr(s1);
	addr = ntohl(addr);
	printf("%lld\n", addr);

	//keep listening for data    
	while(1)    
	{        
		printf("Waiting for data...\n");
		fflush(stdout);        

		if(host != 0)
		{	

			//try to receive some data, this is a blocking call        
			if ((recv_len = recvfrom(sockfd, &packet, (1024 + sizeof(packet)), 0, (struct sockaddr *) &si_other, &slen)) == -1)        
			{            
				die("recvfrom()");        
			}        
		
			//print results
			printf("Received DCHP discover\n");
			print(packet);		
		
			//now reply the client
			addr += 1;
			packet.yiaddr = htonl(addr);	//IP4 + 1;
			packet.lifetime = 3600;
       
			//send data
			if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*) &si_other, slen) ==-1)        
			{            
				die("sendto()");        
			}
		
			//try to receive some data, this is a blocking call        
			if ((recv_len = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &si_other, &slen)) == -1)        
			{            
				die("recvfrom()");        
			}  

			//print results
			printf("Received DCHP request\n");
			print(packet);	

				
			//send ACK
			if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*) &si_other, slen) ==-1)        
			{            
				die("sendto()");        
			}

			bzero(&packet, sizeof(packet));
		host--;
		}
		else
		{
			printf("There are no more yiaddr's. Exiting the server.\n");
			exit(1);
		}
		  
	}    

	close(sockfd);    
	return 0;
}
void print(struct dchp_pkt packet)
{
	printf("siaddr: %llu.%llu.%llu.%llu\n", packet.siaddr & 0xFF, (packet.siaddr >> 8) & 0xFF, (packet.siaddr >> 16) & 0xFF, (packet.siaddr >> 24) & 0xFF);  
	printf("yiaddr: %llu.%llu.%llu.%llu\n", packet.yiaddr & 0xFF, (packet.yiaddr >> 8) & 0xFF, (packet.yiaddr >> 16) & 0xFF, (packet.yiaddr >> 24) & 0xFF);	
	  
	printf("transaction ID: %d\n", packet.tran_ID);  
	printf("lifetime: %d secs\n\n", packet.lifetime);
}