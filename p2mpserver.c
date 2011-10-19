#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/socket.h>
#include "p2mpserver.h"

void usage()
{
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
	
	p2mp_sb server;
		

	return 0;
}

