/*    
Asha Wright
Simple udp client
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define SERVER "129.120.151.94"
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
	struct sockaddr_in si_other;    
	int sockfd, i=0, slen=sizeof(si_other);
	char buf[BUFLEN];    
	char message[BUFLEN];    
	srand(time(NULL));

	if(argc < 2)
	{
		fprintf(stderr, "To run: %s port\n", argv[0]);
		exit(1);
	}

	int PORT = atoi(argv[1]);

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)    
	{        
		die("socket");    
	}    

	memset((char *) &si_other, 0, sizeof(si_other));    
	si_other.sin_family = AF_INET;    
	si_other.sin_port = htons(PORT);    

	if (inet_aton(SERVER , &si_other.sin_addr) == 0)     
	{        
		fprintf(stderr, "inet_aton() failed\n");       
		 exit(1);    
	}    

	while(1)    
	{               
		packet.siaddr = inet_addr(SERVER);	// Server IP address
		packet.yiaddr = 0000;         // Your IP address
		packet.tran_ID = rand() % 500;        // Transaction ID
		packet.lifetime = 0;

		//send the message        
		if (sendto(sockfd, &packet, (1024 + sizeof(packet)), 0, (struct sockaddr *) &si_other, slen) == -1)        
		{
            		die("sendto()");        
		}            

		//try to receive some data, this is a blocking call        
		if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &si_other, &slen) == -1)        
		{            
			die("recvfrom()");        
		}        
		//print results to console
		printf("Recieved DHCP offer\n");
		print(packet);

		//set values
		packet.tran_ID += 1;

		//send the message        
		if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &si_other, slen) == -1)        
		{
            		die("sendto()");        
		}   

		//try to receive some data, this is a blocking call        
		if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *) &si_other, &slen) == -1)        
		{            
			die("recvfrom()");        
		}        
		//print results to console
		printf("Recieved DHCP ACK\n");
		print(packet);


		int runAgain;
		printf("Run again? (1 = Yes/2 = No)");
		scanf("%d", &runAgain);
		if(runAgain == 2)
			break;

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