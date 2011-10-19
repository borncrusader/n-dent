#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/socket.h>
#include "p2mpserver.h"
#include <error.h>

void usage() {
	printf("Please specify all the arguments\n");
        printf("p2mpserver port# file-name N p\n");
        printf("port# - Port number to which the server is listening\n");
        printf("file-name - File where the data will be written\n");
        printf("N - Window size\n");
        printf("p - Probability of packet loss\n");
        exit(1);
}

int main(int argc, char *argv[])
{
	if(argc!=5)
	usage();

	p2mp_sb serv;

	if((serv.sock_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("Server Socket creation failed",errno);
		return 1;
	}
	
	serv.server.sin_family = AF_INET;
	serv.server.sin_addr.s_addr = INADDR_ANY;
	serv.server.sin_port= (htons(atoi(argv[1])));	

	if(bind(serv.sock_server, (struct sockaddr*)&serv.server, sizeof(serv.server)) == -1) {
		close(serv.sock_server);
		die("Server bind failed",errno);
		return 1;
	}
	
	close(serv.sock_server);
	return 0;
}

